//
// Created by fzy on 23-5-8.
//

#ifndef TINYNETFLOW_EPOLLPOLLER_H
#define TINYNETFLOW_EPOLLPOLLER_H

#include <vector>
#include <map>
#include <sys/epoll.h>

#include "netflow/OSAdaptor/include/time/Timestamp.h"

namespace netflow::net {

class Channel; /** 前置声明 */
class EventLoop;

class EpollPoller {
public:
    using ChannelLists = std::vector<Channel*>;
    EpollPoller(EventLoop* loop);
    ~EpollPoller();
    netflow::base::Timestamp poll(int timeoutMs, ChannelLists* activeChannels);
    //void addChannel(Channel* channel);
    //void modifyChannel(Channel* channel);
    void updateChannel(Channel* channel); /** 将添加与修改合二为一，使得上层无需关心 */
    void removeChannel(Channel* channel);

    bool hasChannel(Channel* channel);
    void assertInLoopThread();
private:
    void update(int operation, Channel* channel);
    void fillActiveChannel(int numEvents, ChannelLists* activeChannels) const;
    static const char* operationToString(int op);

private:
    static const int kInitEventListSize = 16;
    int epollFd_;
    std::map<int, Channel*> channels_; /** TODO: why map */
    std::vector<struct epoll_event> events_;  //! 自适应
    EventLoop* ownerLoop_;
};

} // namespace netflow::net



#endif //TINYNETFLOW_EPOLLPOLLER_H
