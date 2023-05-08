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
    /*! 在当前loop中执行 */
    void runInLoop(Functor cb);
    /*! 讲上层注册的回调函数cb放回队列中，唤醒loop所在的线程执行cb */
    void queueInLoop(Functor cb);
    size_t queueSize() const;

    //! 定时器时间
    TimerId runAt(Timestamp time, TimerCallback cb);
    TimerId runAfter(double delay, TimerCallback cb);
    TimerId runEvery(double interval, TimerCallback cb);
    void cancel(TimerId timerId);
    /*! 通过 eventfd 唤醒loop所在的线程
     * 原因是对同一个epoll fd的操作（添加，删除，修改，等待）都放在同一个线程中执行 */
    void wakeup();
    /*! 调用 poller 方法 */
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    bool hasChannel(Channel* channel);

    void assertInLoopThread() {
        if (!isInLoopThread()) {
            abortNotInLoopThread();
        }
    }
    /** 判断Eventloop是否在自己的线程里
     * threadId_： Eventloop创建时的线程ID， tid：此时的线程ID */
    bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }
    bool eventHandling() const { return eventHandling_; }

    void setContext(const boost::any& context) { context_ = context; }
    const boost::any& getContext() const { return context_; } //! 返回引用
    boost::any* getMutableContext() { return &context_; }   //! 返回指针

    static EventLoop* getEventLoopOfCurrentThread();

private:
    void abortNotInLoopThread();
    /** 给eventfd返回的文件描述符wakeupFd_绑定的事件回调
     * 当wakeup()时 即有事件发生时 调用handleRead()读wakeupFd_的8字节 同时唤醒阻塞的epoll_wait */
    void handleRead();
    /** 执行上层回调 */
    void doPendingFunctors();
    void printActiveChannels() const;  //! for debug

private:
    using ChannelList = std::vector<Channel*>;
    bool looping_;
    std::atomic<bool> quit_;
    bool eventHandling_;
    bool callingPendingFunctors_;
    int64_t iteration_;
    /** 标识 Eventloop所属的线程 */
    const pid_t threadId_;
    /** poller返回channels时的时间戳 */
    Timestamp pollReturnTime_;
    std::unique_ptr<Poller> poller_;
    std::unique_ptr<TimerQueue> timerQueue_;
    /** eventfd, 当 mainloop 获取到一个新channel后，需要通过轮询算法选择subloop， 通过该成员唤醒subloop处理channel */
    int wakeupFd_;
    std::unique_ptr<Channel> wakeupChannel_;
    boost::any context_;
    /** 返回poller检测到当前有事件发生的channel列表 */
    ChannelList activeChannels_;
    Channel* currentActiveChannel_;

    mutable MutexLock mutex_;
    /** 存储loop需要执行的所有回调操作 */
    std::vector<Functor> pendingFunctors_ GUARDED_BY(mutex_);
};

} // namespace muduo::net

#endif //LIBZV_EVENT_LOOP_H
