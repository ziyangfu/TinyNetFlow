//
// Created by fzy on 23-5-10.
//

#include "EventLoop.h"

#include "TimerQueue.h"
#include "EpollPoller.h"
#include "Channel.h"
#include "SocketsOps.h"
#include "../base/Logging.h"

#include <sys/eventfd.h>

using namespace netflow::net;

/********************************************************************************************/
/** 匿名空间 */
namespace
{
    //__thread EventLoop* t_loopInThisThread = nullptr;  /** 保存当前 EventLoop this指针 */
    /** thread_local 变量，记住一个线程只有一个EventLoop */
    thread_local EventLoop* t_loopInThisThread = nullptr;

    const int kPollTimeMs = 10000;
/** 使用eventfd 对象实现异步唤醒功能 */
    int createEventfd()
    {
        int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        if (evtfd < 0)
        {
            abort();
        }
        return evtfd;
    }
}  // namespace

/********************************************************************************************/


EventLoop::EventLoop()
    : poller_(std::make_unique<EpollPoller>(this)),
      wakeupFd_(createEventfd()),
      wakeupChannel_(std::make_unique<Channel>(this, wakeupFd_)),
      looping_(false),
      quit_(false),
      eventHandling_(false),
      callingPendingFunctors_(false),
      currentActiveChannel_(nullptr),
      timerQueue_(std::make_unique<TimerQueue>(this)),
      iteration_(0),
      tid_(std::this_thread::get_id())
{
    if(t_loopInThisThread) {
        /** 之前已经创建过 EventLoop， 违背了 one loop per thread， 报错 */
        STREAM_FATAL << "another EventLoop " << t_loopInThisThread << " exists in this thread " << tid_;
        abort();
    }
    else {

        t_loopInThisThread = this;  /** 保存 this 指针 */
    }
    /** 设置唤醒机制 */
    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleReadForWakeup, this));
    wakeupChannel_->enableReading();  /** 将event fd 纳入 epoll 监控范围 */
}

EventLoop::~EventLoop() {
    /** 移除 wakeup相关设置 */
    STREAM_TRACE << "dtor: ~EventLoop()";
    wakeupChannel_->disableAll();
    wakeupChannel_->removeChannel();
    ::close(wakeupFd_);
    t_loopInThisThread = nullptr;
}

void EventLoop::loop() {
    looping_ = true;
    quit_ = false;
    /** 无限循环 */
    while (!quit_){
        activeChannels_.clear();
        pollReturnTime_ =  poller_->poll(10000,&activeChannels_);  /**将长期阻塞在这里，等待事件发生 */

        eventHandling_ = true;
        for(Channel* channel : activeChannels_) {
            currentActiveChannel_ = channel;
            currentActiveChannel_->handleEvent(pollReturnTime_);  /** 处理 epoll 发生事件 */
        }
        currentActiveChannel_ = nullptr;
        eventHandling_ = false;
        doPendingFunctors();  /** 处理上层事件 */
    }
    looping_ = false;
}

void EventLoop::quit() {
    quit_ = true;

    /** IO相关的事情，在IO线程中处理 */
    if (!isInLoopThread()) {
        wakeup();
    }
}
/** EventLoop 充当 Channel 与 EpollPoller 的连接纽带 */
void EventLoop::updateChannel(netflow::net::Channel *channel) {
    assert(channel->getOwnerLoop() == this);
    assertInLoopThread();
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(netflow::net::Channel* channel) {
    assert(channel->getOwnerLoop() == this);
    assertInLoopThread();
    if (eventHandling_) {
        assert(currentActiveChannel_ == channel ||
        (std::find(activeChannels_.begin(), activeChannels_.end(), channel)) == activeChannels_.end() );
    }
    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(netflow::net::Channel *channel) {
    assert(channel->getOwnerLoop() == this);
    assertInLoopThread();
    return poller_->hasChannel(channel);
}
/**
 * \brief 唤醒IO线程，即将EventLoop从loop函数的poll中唤醒，使得loop循环可以去处理 doPendingFunctors
 * */
void EventLoop::wakeup() {
    uint64_t one = 1;
    ssize_t n = sockets::write(wakeupFd_, &one, sizeof one);
    if (n != sizeof one)
    {
        STREAM_ERROR << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
    }
}

void EventLoop::handleReadForWakeup() {
    uint64_t one = 1;
    ssize_t n = sockets::read(wakeupFd_, &one, sizeof one); /** SocketsOps封装 */
    if (n != sizeof one)
    {
        /** 出错啦 */
        STREAM_ERROR << "wakeup failed! ";
    }
}

void EventLoop::runInLoop(Functor cb) {
    if(isInLoopThread()){
        cb();
    }
    else {
        queueInLoop(std::move(cb));
    }

}

void EventLoop::queueInLoop(Functor cb) {
    std::unique_lock<std::mutex> lock(mutex_);
    pendingFunctors_.push_back(std::move(cb));
    lock.unlock();
    if((!isInLoopThread()) || callingPendingFunctors_) {
        wakeup();
    }

}
/*!
 * \brief 执行上层回调 */
void EventLoop::doPendingFunctors() {
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;
    /** 为了防止阻塞，交换 */
    {
        std::unique_lock<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }

    /** 处理回调 */
    for (const Functor& functor : functors)
    {
        functor();
    }
    callingPendingFunctors_ = false;
}

TimerId EventLoop::runAt(netflow::base::Timestamp time, netflow::net::TimerCallback cb) {
    return timerQueue_->addTimer(std::move(cb), time,  0.0);
}

TimerId EventLoop::runAfter(double delay, netflow::net::TimerCallback cb) {
    Timestamp time(addTime(Timestamp::now(), delay));
    return runAt(time, std::move(cb));
}

TimerId EventLoop::runEvery(double interval, netflow::net::TimerCallback cb) {
    Timestamp time(addTime(Timestamp::now(), interval));
    return timerQueue_->addTimer(std::move(cb), time, interval);
}

void EventLoop::cancel(netflow::net::TimerId timerId) {
    return timerQueue_->cancel(timerId);
}

EventLoop *EventLoop::getEventLoopOfCurrentThread() {
    return t_loopInThisThread;
}

void EventLoop::abortNotInLoopThread() {
    STREAM_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
                 << " was created in threadId_ = "  << tid_
                 << ", current thread id = " << std::this_thread::get_id();
}