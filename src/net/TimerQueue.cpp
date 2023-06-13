//
// Created by fzy on 23-5-31.
//

#include "TimerQueue.h"

#include "EventLoop.h"
#include "Timer.h"
#include "TimerId.h"
#include "../base/Logging.h"

#include <sys/timerfd.h>  /** only Linux, non POSIX */
#include <unistd.h>
#include <assert.h>

/** TODO: 跨平台定时器开发 */

namespace netflow::net::detail {

int createTimerfd() {
    /**  CLOCK_MONOTONIC 单调时钟
    timerfd_create（）函数创建一个定时器对象，同时返回一个与之关联的文件描述符。
    clockid：clockid标识指定的时钟计数器，可选值（CLOCK_REALTIME、CLOCK_MONOTONIC。。。）
    CLOCK_REALTIME:系统实时时间,随系统实时时间改变而改变,即从UTC1970-1-1 0:0:0开始计时,中间时刻如果系统时间被用户改成其他,则对应的时间相应改变
    CLOCK_MONOTONIC:从系统启动这一刻起开始计时,不受系统时间被用户改变的影响
    flags：参数flags（TFD_NONBLOCK(非阻塞模式)/TFD_CLOEXEC（表示当程序执行exec函数时本fd将被系统自动关闭,表示不传递）
*/
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerfd < 0) {
        STREAM_FATAL << "timerfd create failed.";
    }
    return timerfd;
}
/*! TODO */
struct timespec howMuchTimeFromNow(Timestamp when) {
    int64_t microseconds = when.microSecondsSinceEpoch()
                           - Timestamp::now().microSecondsSinceEpoch();
    if (microseconds < 100)
    {
        microseconds = 100;
    }
    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(
            microseconds / Timestamp::kMicroSecondsPerSecond);
    ts.tv_nsec = static_cast<long>(
            (microseconds % Timestamp::kMicroSecondsPerSecond) * 1000);
    return ts;
}

void readTimerfd(int timerfd, Timestamp now) {
    uint64_t howmany;
    /** 读取计时器的超时次数 */
    ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
    STREAM_TRACE << "TimerQueue::handleRead() " << howmany << " at " << now.toString();
    if (n != sizeof howmany)
    {
        STREAM_ERROR << "TimerQueue::handleRead() reads " << n << " bytes instead of 8";
    }
}
/*!
 * \brief 开始计时
 * \param timerfd: timer描述符
 * \param expiration: 到期时间 */
void resetTimerfd(int timerfd, Timestamp expiration) {
    /** struct itimerspec {
            struct timespec it_interval;  // Interval for periodic timer （定时间隔周期）
            struct timespec it_value;  //   Initial expiration (第一次超时时间)
         }
     */
    struct itimerspec newValue = {0};
    struct itimerspec oldValue = {0};
    newValue.it_value = howMuchTimeFromNow(expiration);
    /**
     timerfd_settime()此函数用于设置新的超时时间，并开始计时,能够启动和停止定时器;
     fd: 参数fd是timerfd_create函数返回的文件句柄
     flags：1代表设置的是绝对时间（TFD_TIMER_ABSTIME 表示绝对定时器）；为0代表相对时间。
     new_value: 参数new_value指定定时器的超时时间以及超时间隔时间
     old_value: 如果old_value不为NULL, old_vlaue返回之前定时器设置的超时时间，具体参考timerfd_gettime()函数

     ** it_interval不为0则表示是周期性定时器。
        it_value和it_interval都为0表示停止定时器
 */
    int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
    if (ret) {
        STREAM_ERROR << "timerfd_settime() error";
    }
}

}  // namespace netflow::net::detail

using namespace netflow::net::detail;
using namespace netflow::net;

TimerQueue::TimerQueue(netflow::net::EventLoop *loop)
    : loop_(loop),
      timerfd_(createTimerfd()),
      timerfdChannel_(loop, timerfd_),
      timers_(),
      callingExpiredTimers_(false)
{
    timerfdChannel_.setReadCallback(std::bind(&TimerQueue::handleRead, this));
    timerfdChannel_.enableReading();
}

TimerQueue::~TimerQueue() {
    timerfdChannel_.disableAll(); /** epoll处理 */
    timerfdChannel_.removeChannel(); /** channel 处理 */
    ::close(timerfd_);

    for (const Entry& timer : timers_) {
        delete timer.second;  /** 为什么只删除第二个, Timer在堆区，手动释放，Timestamp在栈区，自动释放  */
    }
}
/*!
 * \brief 添加定时器
 * \param cb: 定时器回调函数
 * \param when: 到期时间
 * \param interval: 周期时间 */
TimerId TimerQueue::addTimer(netflow::net::TimerCallback cb, netflow::base::Timestamp when, double interval) {
    STREAM_TRACE << "addTimer()";
    Timer* timer = new Timer(std::move(cb), when, interval);
    loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, timer));
    return TimerId(timer, timer->sequence());
}

void TimerQueue::addTimerInLoop(netflow::net::Timer *timer) {
    STREAM_TRACE << "addTimerInLoop()";
    loop_->assertInLoopThread();
    bool earlistChanged = insert(timer);
    if (earlistChanged) {
        resetTimerfd(timerfd_, timer->getExpiration());
    }
}

void TimerQueue::cancel(netflow::net::TimerId timerId) {
    loop_->runInLoop(std::bind(&TimerQueue::cancelInLoop, this, timerId));
}

void TimerQueue::cancelInLoop(netflow::net::TimerId timerId) {
    loop_->assertInLoopThread();
    assert(timers_.size() == activeTimers_.size());
    ActiveTimer timer(timerId.timer_, timerId.sequence_);
    ActiveTimerSet::iterator it = activeTimers_.find(timer);
    /** 找到了 */
    if (it != activeTimers_.end()) {
        size_t n = timers_.erase(Entry(it->first->getExpiration(), it->first));
        (void)n;
        assert(n == 1);

        delete it->first;  /** delete 在堆区的 timer */
        activeTimers_.erase(it);

    }
    /** 此时正在执行 Timer 回调，将该需要删除的timer暂时存在cancelingTimers_中，后续删除 */
    else if (callingExpiredTimers_) {
        cancelingTimers_.insert(timer);
    }
    assert(timers_.size() == activeTimers_.size());
}
/*!
 * \brief timer到期时，timerfd 触发读事件，读事件处理函数， IO线程处理 */
void TimerQueue::handleRead() {
    loop_->assertInLoopThread();
    Timestamp now(Timestamp::now());
    readTimerfd(timerfd_, now);  /** TODO： 作用是什么？ */

    std::vector<Entry> expired = getExpired(now);

    callingExpiredTimers_ = true;

    for (const Entry& it : expired) {
        it.second->run();   /** 运行定时器回调函数 */
    }
    callingExpiredTimers_ = false;
    reset(expired, now);
}
/*!
 * \brief 获取到期的定时器， 移除到期的Timer，并通过vector返回它们 */
std::vector<TimerQueue::Entry> TimerQueue::getExpired(netflow::base::Timestamp now) {
    assert(timers_.size() == activeTimers_.size());
    std::vector<Entry> expired;
    /** INTPTR_MAX：表示 intptr_t 类型的最大值
     * 最长的定时， sentry：哨兵值 */
    Entry sentry(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
    /** 返回第一个未到期的Timer的迭代器 */
    TimerList::iterator end = timers_.lower_bound(sentry);  /** lower_bound： 返回指向首个不小于给定键的元素的迭代器 */
    /** 最后一个或者没到期 */
    assert(end == timers_.end() || now < end->first);
    /** 从timers_中拷贝到 expired */
    std::copy(timers_.begin(), end, std::back_inserter(expired));
    timers_.erase(timers_.begin(), end);

    for (const Entry& it : expired) {
        ActiveTimer timer(it.second, it.second->sequence());
        size_t n = activeTimers_.erase(timer);
        (void)n;
        assert(n == 1);
    }
    assert(timers_.size() == activeTimers_.size());
    return expired;
}
/*!
 * \brief 重新计时 */
void TimerQueue::reset(const std::vector<Entry> &expired, netflow::base::Timestamp now) {
    Timestamp  nextExpire;
    for (const Entry& it : expired) {
        ActiveTimer timer(it.second, it.second->sequence());
        if (it.second->repeat()
            && (cancelingTimers_.find(timer) == cancelingTimers_.end()) ) {
            it.second->restart(now);
            insert(it.second);
        }
        else {
            delete it.second;
        }
    }
    if (!timers_.empty()) {
        nextExpire = timers_.begin()->second->getExpiration();
    }

    if (nextExpire.valid()) {
        resetTimerfd(timerfd_, nextExpire);
    }
}

/*!
 * 插入条件是什么？ */
bool TimerQueue::insert(netflow::net::Timer *timer) {
    loop_->assertInLoopThread();
    assert(timers_.size() == activeTimers_.size());
    bool earliestChanged = false;
    Timestamp when = timer->getExpiration();
    TimerList::iterator it = timers_.begin();
    /** 容器为空或到期时间比第一个小 */
    if (it == timers_.end() || when < it->first) {
        earliestChanged = true;
    }

    {
        std::pair<TimerList::iterator, bool> result = timers_.insert(Entry(when, timer));
        assert(result.second);
        (void) result;
    }

    {
        std::pair<ActiveTimerSet::iterator, bool> result =
                activeTimers_.insert(ActiveTimer(timer, timer->sequence()));
        assert(result.second);
        (void) result;
    }
    assert(timers_.size() == activeTimers_.size());
    return earliestChanged;
}
