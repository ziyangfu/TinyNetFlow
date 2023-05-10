//
// Created by fzy on 23-5-10.
//

#ifndef TINYNETFLOW_EVENTLOOP_H
#define TINYNETFLOW_EVENTLOOP_H
#include <atomic>
#include <mutex>
#include <memory>
#include <vector>

namespace netflow::net {

class Channel;
class EpollPoller;

class EventLoop {
public:
    EventLoop();
    ~EventLoop();
    void loop();
    void quit();
    void runInLoop();
    void queueInLoop();
    std::size_t getQueueSize() const;

    void wakeup();
    void addChannel(Channel* channel);
    void removeChannel(Channel channel);
    void modifyChannel(Channel channel);
    bool hasChannel(Channel* channel);


private:
    void abortNotInLoopThread();

    void handleReadForWakeup();
    void doPendingFunctors();
private:
    using ChannelList = std::vector<Channel*>;
    std::atomic_bool looping_;
    std::atomic_bool quit_;
    std::atomic_bool eventHandling_;
    std::atomic_bool callingPendingFunctors_;

    int wakeupId_;

    std::unique_ptr<EpollPoller> poller_;
    std::unique_ptr<Channel> wakeupChannel_;
    ChannelList activeChannels_;
    Channel* currentActiveChannel_;

    std::mutex mutex_;



};

} // namespace netflow::net
#endif //TINYNETFLOW_EVENTLOOP_H
