//
// Created by fzy on 23-5-10.
//

#ifndef TINYNETFLOW_EVENTLOOP_H
#define TINYNETFLOW_EVENTLOOP_H
#include <atomic>
#include <mutex>
#include <memory>
#include <vector>
#include <functional>

namespace netflow::net {

class Channel;
class EpollPoller;

class EventLoop {
public:
    using Functor = std::function<void()>; /** 上层回调函数 */
    EventLoop();
    ~EventLoop();
    void loop();
    void quit();
    void runInLoop(Functor cb);
    void queueInLoop(Functor cb);
    bool isInLoopThead();
    std::size_t getQueueSize() const;

    void wakeup();  /** 通过 event fd 唤醒 */
    void addChannel(Channel* channel);
    void removeChannel(Channel channel);
    void modifyChannel(Channel channel);
    bool hasChannel(Channel* channel);

    void assertInLoopThread()
    {
        if (!isInLoopThread())
        {
            abortNotInLoopThread();
        }
    }

private:
    void abortNotInLoopThread();

    void handleReadForWakeup();
    void doPendingFunctors();
private:
    std::atomic_bool looping_;
    std::atomic_bool quit_;
    std::atomic_bool eventHandling_;
    std::atomic_bool callingPendingFunctors_;

    int wakeupFd_;

    std::unique_ptr<EpollPoller> poller_;
    std::unique_ptr<Channel> wakeupChannel_;
    using ChannelList = std::vector<Channel*>;
    ChannelList activeChannels_;
    Channel* currentActiveChannel_;

    std::mutex mutex_;
    std::vector<Functor> pendingFunctors_;

    EventLoop* m_loopInThisThread = nullptr;



};

} // namespace netflow::net
#endif //TINYNETFLOW_EVENTLOOP_H
