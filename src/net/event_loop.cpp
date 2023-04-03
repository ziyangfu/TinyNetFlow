//
// Created by fzy on 23-3-10.
//

#include "event_loop.h"

#include "../base/logging.h"
#include "../base/mutex.h"
#include "channel.h"
#include "poller.h"
#include "socket_ops.h"
#include "timer_queue.h"

#include <algorithm>
#include <signal.h>
#include <sys/eventfd.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;
//! 匿名 namespace
//! https://blog.csdn.net/Solstice/article/details/6186978
namespace {
__thread EventLoop* t_loopInThisThread = 0;

const int kPollTimeMs = 10000;
/** 使用eventfd 对象实现异步唤醒功能 */
int createEventfd() {
    int evtfd = eventfd(0,EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0) {
        LOG_SYSERR << "Failed in eventfd";
        abort();
    }
    return evtfd;
}

#pragma GCC diagnostic ignored "-Wold-style-cast"
    class IgnoreSigPipe
    {
    public:
        IgnoreSigPipe()
        {
            ::signal(SIGPIPE, SIG_IGN);
            // LOG_TRACE << "Ignore SIGPIPE";
        }
    };
#pragma GCC diagnostic error "-Wold-style-cast"

IgnoreSigPipe initObj;
}  // 匿名 namespace

EventLoop* EventLoop::getEventLoopOfCurrentThread() {
    return t_loopInThisThread;
}

EventLoop::EventLoop()
    : looping(false),
      quit_(false),
      eventHandling_(false),
      callingPendingFunctors_(false),
      iteration_(0),
      threadId_(CurrentThread::tid()),
      poller_(Poller::newDefaultPoller(this)),
      timeQueue_(std::make_unique<TimerQueue>(this)),    // timerQueue_(new TimerQueue(this)),
      wakeupFd_(createEventfd()),
      wakeupChannel_(std::make_unique<Channel>(this, wakeupFd_)),
      currentActiveChannel_(NULL)
{
// PASS
}

EventLoop::~EventLoop() {
    LOG_DEBUG << "EventLoop " << this << " of thread " << threadId_
              << " destructs in thread " << CurrentThread::tid();
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    ::close(wakeupFd_);
    t_loopInThisThread = NULL;
}

void EventLoop::loop() {

}

void EventLoop::quit() {
    quit_ = true;
    if (!isInLoopThread()) {
        wakeup();
    }
}


void EventLoop::runInLoop(Functor cb) {

}

void EventLoop::queueInLoop(muduo::net::EventLoop::Functor cb) {

}

size_t EventLoop::queueSize() const {

}

TimerId EventLoop::runAt(muduo::Timestamp time, muduo::net::TimerCallback cb) {

}

TimerId EventLoop::runAfter(double delay, muduo::net::TimerCallback cb) {

}

TimerId EventLoop::runEvery(double interval, muduo::net::TimerCallback cb) {

}

void EventLoop::cancel(muduo::net::TimerId timerId) {

}

void EventLoop::updateChannel(muduo::net::Channel *channel) {

}

void EventLoop::removeChannel(muduo::net::Channel *channel) {

}

void EventLoop::hasChannel(muduo::net::Channel *channel) {

}

void EventLoop::abortNotInLoopThread() {

}

void EventLoop::wakeup() {

}

void EventLoop::handleRead() {

}

void EventLoop::doPendingFunctors() {

}

void EventLoop::printActiveChannels() const {

}




