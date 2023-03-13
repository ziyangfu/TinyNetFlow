//
// Created by fzy on 23-3-10.
//

/*! 事件循环 */
#ifndef LIBZV_EVENT_LOOP_H
#define LIBZV_EVENT_LOOP_H

#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <memory>
#include <unordered_map>

#include <atomic>

#include <boost/any.hpp>  // only for boost::any


namespace libzv::net {

class Channel;
class Poller;
class TimeQueue;

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
    void runInLoop(Functor cb);
    void queueInLoop(Functor cb);

    void isInLoopThread() const {
        //return threadId_ = getpid();
    };

    void wakeup();
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    void hasChannel(Channel* channel);

private:
    void abortNotInLoopThread();
    void handleRead();

private:
    using ChannelList = std::vector<Channel*>;
    std::atomic_bool loop_, quit_, eventHandling_, callingPendingFunctors_;
    int64_t iteration_;
    const pid_t threadId_; // TODO 初始化
    int wakeupFd_;
    boost::any context_;

    std::unique_ptr<Poller> poller_;
    std::unique_ptr<Channel> wakeupChannel_;
    std::unique_ptr<TimeQueue> timeQueue_;





};

} // libzv::net

#endif //LIBZV_EVENT_LOOP_H
