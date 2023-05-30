//
// Created by fzy on 23-5-8.
//
#include <sys/epoll.h>
#include <strings.h>

#include "EpollPoller.h"
#include "Channel.h"
#include "EventLoop.h"

#include "../base/Logging.h"

using namespace netflow::base;
using namespace netflow::net;

EpollPoller::EpollPoller(EventLoop* loop)
    :epollFd_(::epoll_create1(EPOLL_CLOEXEC))
{

}

EpollPoller::~EpollPoller() {
    ::close(epollFd_);
}

netflow::base::Timestamp EpollPoller::poll(int timeoutMs, ChannelLists* activeChannels) {
    STREAM_TRACE << "fd total count " << channels_.size();
    int numActiveEvents = ::epoll_wait(epollFd_, &(*events_.begin()),
                                       static_cast<int>(events_.size()),
                                       timeoutMs);
    Timestamp now(Timestamp::now());
    if(numActiveEvents > 0) {
        STREAM_TRACE << numActiveEvents << " events happened";
        //! 将活动的channel push到 activeChannels容器
        fillActiveChannel(numActiveEvents, activeChannels);
        //! 2倍扩容， 目前缺陷，只能扩容，不能缩
        if (static_cast<size_t>(numActiveEvents) == events_.size()) {
            events_.resize(events_.size() * 2);
        }
    }
    else if (numActiveEvents == 0) {
        // 无事发生
        STREAM_TRACE << "nothing happened";
    }
    else {
        // 出错
        STREAM_ERROR << "epoll::poll() error";

    }
    return now;
}

void EpollPoller::addChannel(Channel *channel) {
    update(EPOLL_CTL_ADD, channel);
}

void EpollPoller::removeChannel(Channel *channel) {
    update(EPOLL_CTL_DEL, channel);
    // del channel
}

void EpollPoller::modifyChannel(Channel *channel) {
    update(EPOLL_CTL_MOD, channel);
}

void EpollPoller::update(int operation, netflow::net::Channel *channel) {
    struct epoll_event event;
    bzero(&event, sizeof event);
    event.events = channel->getEvents();
    int fd = channel->getFd();

    ::epoll_ctl(epollFd_, operation, fd, &event);
}



void EpollPoller::fillActiveChannel(int numEvents, EpollPoller::ChannelLists *activeChannels) const {
    for(int i = 0; i < numEvents; i++) {
        Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
        channel->setEvents(events_[i].events);
        activeChannels->push_back(channel);
    }
}
