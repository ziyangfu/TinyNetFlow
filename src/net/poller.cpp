//
// Created by fzy on 23-3-31.
//

#include "poller.h"
#include "channel.h"

using namespace muduo;
using namespace muduo::net;

Poller::Poller(muduo::net::EventLoop *loop)
    : ownerLoop_(loop)
{}
Poller::~Poller() = default;

bool Poller::hasChannel(muduo::net::Channel *channel) const {
    assertInLoopThread();
    ChannelMap::const_iterator it = channels_.find(channel->fd());  //! 查找
    //! 查到了，并且 value 就是对应的 channel
    return it != channels_.end() && it->second == channel;
}
