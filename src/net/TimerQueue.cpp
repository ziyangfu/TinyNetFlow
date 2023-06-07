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
    /**  CLOCK_MONOTONIC 单调时钟 */
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerfd < 0) {
        STREAM_FATAL << "timerfd create failed.";
    }
    return timerfd;
}
/*! TODO */
struct timespec howMuchTimeFromNow(Timestamp now) {
    struct timespec ts = {0};
    return ts;
}

void readTimerfd(int timerfd, Timestamp now) {}

void resetTimerfd(int timerfd, Timestamp expiration) {
    struct itimerspec newValue = {0};
    struct itimerspec oldValue = {0};
    newValue.it_value = howMuchTimeFromNow(expiration);
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
        delete timer.second;  /** 为什么只删除第二个 */
    }
}

TimerId TimerQueue::addTimer(netflow::net::TimerCallback cb, netflow::base::Timestamp when, double interval) {
    Timer* timer = new Timer(std::move(cb), when, interval); /** when delete ?? */
    loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, timer));
    return TimerId(timer, timer->sequence());

}

void TimerQueue::addTimerInLoop(netflow::net::Timer *timer) {
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
    
}

void TimerQueue::handleRead() {

}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(netflow::base::Timestamp now) {

}

void TimerQueue::reset(const std::vector<Entry> &expired, netflow::base::Timestamp now) {

}
/*!
 * 插入条件是什么？ */
bool TimerQueue::insert(netflow::net::Timer *timer) {
    loop_->assertInLoopThread();
    assert(timers_.size() == activeTimers_.size());
    bool earliestChanged = false;
    Timestamp when = timer->getExpiration();
    TimerList::iterator it = timers_.begin();
    /** 插入条件是什么？ */
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
                activeTimers_.insert(ActiveEntry(timer, timer->sequence()));
        assert(result.second);
        (void) result;
    }
    assert(timers_.size() == activeTimers_.size());
    return earliestChanged;
}
