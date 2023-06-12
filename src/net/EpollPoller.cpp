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
    : epollFd_(::epoll_create1(EPOLL_CLOEXEC)),
      events_(kInitEventListSize),
      ownerLoop_(loop)
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
    /** 将channel 添加到 channels */
    int fd = channel->getFd();
    assert(channels_.find(fd) == channels_.end());
    channels_[fd] = channel;
    update(EPOLL_CTL_ADD, channel);
}

void EpollPoller::removeChannel(Channel *channel) {
    /** 在确定channels_中有channel的前提下，删除channel */
    int fd = channel->getFd();
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    channels_.erase(fd);

    update(EPOLL_CTL_DEL, channel);
}

void EpollPoller::modifyChannel(Channel *channel) {
    update(EPOLL_CTL_MOD, channel);
}

void EpollPoller::update(int operation, netflow::net::Channel *channel) {
    struct epoll_event event;
    bzero(&event, sizeof event);
    event.events = channel->getEvents();
    event.data.ptr = channel;
    int fd = channel->getFd();
    STREAM_TRACE << "Epoll control operation = " << operationToString(operation)
                 << " fd = " << fd;
    if (::epoll_ctl(epollFd_, operation, fd, &event) < 0) {
        STREAM_ERROR  << "Epoll control operation = " << operationToString(operation)
                     << " fd = " << fd;
    }
}

void EpollPoller::fillActiveChannel(int numEvents, EpollPoller::ChannelLists *activeChannels) const {
    for(int i = 0; i < numEvents; i++) {
        Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
        channel->setEvents(events_[i].events);
        activeChannels->push_back(channel);
    }
}

const char *EpollPoller::operationToString(int op) {
    switch (op)
    {
        case EPOLL_CTL_ADD:
            return "ADD";
        case EPOLL_CTL_DEL:
            return "DEL";
        case EPOLL_CTL_MOD:
            return "MOD";
        default:
            assert(false && "ERROR op");
            return "Unknown Operation";
    }
}

void EpollPoller::assertInLoopThread() {
    ownerLoop_->assertInLoopThread();
}

bool EpollPoller::hasChannel(netflow::net::Channel *channel) {
    assertInLoopThread();
    int fd = channel->getFd();
    auto it = channels_.find(fd);
    return it !=channels_.end() &&  it->second == channel;
}

