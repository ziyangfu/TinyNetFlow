//
// Created by fzy on 23-3-10.
//

/*! 事件循环 */
#ifndef LIBZV_EVENT_LOOP_H
#define LIBZV_EVENT_LOOP_H

#include <atomic>
#include <functional>
#include <vector>

#include <boost/any.hpp>  // only for boost::any   #include<any>

#include "../base/mutex.h"
#include "../base/current_thread.h"
#include "../base/timestamp.h"

#include "callback.h"
#include "timer_id.h"

namespace muduo::net {

class Channel;  //! 前向声明，因此可以不用包含对应头文件
class Poller;
class TimerQueue;

class EventLoop : Noncopyable
{
public:
    EventLoop();
    ~EventLoop();

    using Functor = std::function<void()>;
    /*! 事件循环 */
    void loop();
    /*! 停止事件循环 */
    void quit();

    Timestamp pollReturnTime() const { return pollReturnTime_; }

    int64_t iteration() const { return iteration_; }

    void runInLoop(Functor cb);
    void queueInLoop(Functor cb);
    size_t queueSize() const;

    //! 定时器时间
    TimerId runAt(Timestamp time, TimerCallback cb);
    TimerId runAfter(double delay, TimerCallback cb);
    TimerId runEvery(double interval, TimerCallback cb);
    void cancel(TimerId timerId);
    //! 内部使用
    void wakeup();
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    bool hasChannel(Channel* channel);

    void assertInLoopThread() {
        if (!isInLoopThread()) {
            abortNotInLoopThread();
        }
    }
    bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }
    bool eventHandling() const { return eventHandling_; }

    void setContext(const boost::any& context) { context_ = context; }
    const boost::any& getContext() const { return context_; } //! 返回引用
    boost::any* getMutableContext() { return &context_; }   //! 返回指针

    static EventLoop* getEventLoopOfCurrentThread();

private:
    void abortNotInLoopThread();
    //! 唤醒
    void handleRead();
    void doPendingFunctors();
    void printActiveChannels() const;  //! for debug

private:
    using ChannelList = std::vector<Channel*>;
    bool looping_;
    std::atomic<bool> quit_;
    bool eventHandling_;
    bool callingPendingFunctors_;
    int64_t iteration_;
    const pid_t threadId_; // TODO 初始化
    Timestamp pollReturnTime_;
    std::unique_ptr<Poller> poller_;
    std::unique_ptr<TimerQueue> timerQueue_;
    int wakeupFd_;
    std::unique_ptr<Channel> wakeupChannel_;
    boost::any context_;

    ChannelList activeChannels_;
    Channel* currentActiveChannel_;

    mutable MutexLock mutex_;
    std::vector<Functor> pendingFunctors_ GUARDED_BY(mutex_);
};

} // namespace muduo::net

#endif //LIBZV_EVENT_LOOP_H
