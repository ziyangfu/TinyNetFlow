//
// Created by fzy on 23-5-8.
//
#include <sys/epoll.h>
#include <strings.h>
#include "EpollPoller.h"
#include "../base/Logger.h"

using namespace netflow::base;
using namespace netflow::net;

EpollPoller::EpollPoller()
    :epollFd_(::epoll_create1(EPOLL_CLOEXEC))
{

}

EpollPoller::~EpollPoller() {
    ::close(epollFd_);
}

netflow::base::Timestamp EpollPoller::poll(int timeoutMs, ChannelLists* activeChannels) {
    LOG_TRACE("fd total count {}", channels.size());
    spdlog::info("fd {}", channels_.size());
    int numActiveEvents = ::epoll_wait(epollFd_, &(*events_.begin()),
                                       static_cast<int>(events_.size()),
                                       timeoutMs);
    Timestamp now(Timestamp::now());
    if(numActiveEvents > 0) {
        //! 将活动的channel push到 activeChannels容器
        fillActiveChannel(numActiveEvents, activeChannels);
        //! 2倍扩容， 目前缺陷，只能扩容，不能缩
        if (static_cast<size_t>(numActiveEvents) == events_.size()) {
            events_.resize(events_.size() * 2);
        }
    }
    else if (numActiveEvents == 0) {
        // 无事发生
        LOG_INFO()
    }
    else {
        // 出错
    }
    return now;
}

void EpollPoller::addChannel(netflow::Channel *channel) {
    update(EPOLL_CTL_ADD, channel);
}

void EpollPoller::removeChannel(netflow::Channel *channel) {
    update(EPOLL_CTL_DEL, channel);
    // del channel
}

void EpollPoller::modifyChannel(netflow::Channel *channel) {
    update(EPOLL_CTL_MOD, channel);
}

void EpollPoller::update(int operation, netflow::Channel *channel) {
    struct epoll_event event;
    bzero(&event, sizeof event);
    event.events = channel->events();
    int fd = channel->getFd();

    ::epoll_ctl(epollFd_, operation, fd, &event);
}

void EpollPoller::fillActiveChannel(int numEvents, netflow::EpollPoller::ChannelLists *activeChannels) const {
    for(int i = 0; i < numEvents; i++) {
        Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
        channel->setActiveEvents(events_[i].events);
        activeChannels->push_back(channel);
    }
}
