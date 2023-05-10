//
// Created by fzy on 23-5-8.
//

#ifndef TINYNETFLOW_EPOLLPOLLER_H
#define TINYNETFLOW_EPOLLPOLLER_H

#include <vector>
#include <map>
#include <sys/epoll.h>

namespace netflow::net {

class Channel; /** 前置声明 */
class EpollPoller {
public:
    using ChannelLists = std::vector<Channel*>;
    EpollPoller();
    ~EpollPoller();
    void poll(int timeoutMs, ChannelLists* activeChannels);
    void addChannel(Channel* channel);
    void removeChannel(Channel* channel);
    void modifyChannel(Channel* channel);
    void isInLoopThread();
private:
    void update(int operation, Channel* channel);
    void fillActiveChannel(int numEvents, ChannelLists* activeChannels) const;

private:
    static const int kInitEventListSize = 16;
    int epollFd_;
    std::map<int, Channel*> channels_;
    std::vector<struct epoll_event> events_;  //! 自适应
};

} // namespace netflow::net



#endif //TINYNETFLOW_EPOLLPOLLER_H
