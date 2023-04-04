/*!
 * IO 多路复用后端
 * 功能 ： epoll 封装 */

#ifndef LIBZV_EPOLL_POLLER_H
#define LIBZV_EPOLL_POLLER_H

#include "../poller.h"
#include <vector>

struct epoll_event;
namespace muduo {
namespace net {

class EpollPoller :  public Poller {
public:
    EpollPoller(EventLoop* loop);
    ~EpollPoller() override;

    Timestamp poll(int timeoutMs, ChannelList* activeChannels) override;
    void updateChannel(Channel* channel) override;
    void removeChannel(Channel* channel) override;
private:

    static const char* operationToString(int op);

    void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;
    void update(int operation, Channel* channel);
private:
    static const int kInitEventListSize = 16;
    using EventList = std::vector<struct epoll_event>;
    int epollfd_;
    EventList events_;
};

} // namespace net
} // namespace muduo

#endif //LIBZV_EPOLL_POLLER_H
