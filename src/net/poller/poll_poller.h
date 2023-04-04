//
// Created by fzy on 23-3-13.
//

/*! IO 多路复用的接口: poll */

#ifndef LIBZV_POLL_POLLER_H
#define LIBZV_POLL_POLLER_H

#include "../poller.h"
#include <vector>

struct pollfd;

namespace muduo {
namespace net {

class PollPoller : public Poller {
public:
    PollPoller(EventLoop* loop);
    ~PollPoller() override;

    Timestamp poll(int timeoutMs, ChannelList* activeChannels) override;
    void updateChannel(Channel* channel) override;
    void removeChannel(Channel* channel) override;
private:
    void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;

    using PollFdList = std::vector<struct pollfd>;
    PollFdList pollfds_;
};
} // namespace net
} // namespace muduo

#endif //LIBZV_POLL_POLLER_H
