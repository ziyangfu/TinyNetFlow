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

namespace {
    const int kNew = -1;  /** kNew: 未添加的channel */
    const int kAdded = 1; /** kAdded: 已经添加的channel */
    const int kDeleted = 2; /** kDeleted: 已经删除的channel */
}

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

void EpollPoller::fillActiveChannel(int numEvents, EpollPoller::ChannelLists *activeChannels) const {
    for(int i = 0; i < numEvents; i++) {
        Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
        channel->setEvents(events_[i].events);
        activeChannels->push_back(channel);
    }
}

void EpollPoller::updateChannel(netflow::net::Channel *channel) {
    assertInLoopThread();
    const int index = channel->getIndex();
    int fd = channel->getFd();
    STREAM_TRACE << "fd = " << fd << " events = " << channel->getEvents()
                 << " index = " << index;
    /** 未添加的，或者已经删除的，有添加的需求 */
    if (index == kNew || index == kDeleted) {
        if (index == kNew) {
            assert(channels_.find(fd) == channels_.end());
            channels_[fd] = channel;
        }
        else {
            /** epoll中已经删除了，但 channels中还有，需要重新添加到epoll中 */
            assert(channels_.find(fd) != channels_.end());
            assert(channels_[fd] == channel);
        }
        channel->setIndex(kAdded);
        update(EPOLL_CTL_ADD, channel);
    }
    /** 已经添加的，有修改的需求 */
    else {
        /** 一个类型为 void 的表达式，技巧，为了避免变量未被使用的编译器警告。因为如果变量 fd 没有被使用，某些编译器会发出警告 */
        (void)fd;
        assert(channels_.find(fd) != channels_.end());
        assert(channels_[fd] == channel);
        assert(index == kAdded);
        if (channel->isNoneEvent()) {
            /** 若channel关闭读，关闭写，那么将channel从epoll中移除，只保留在channels_ */
            update(EPOLL_CTL_DEL, channel);
            channel->setIndex(kDeleted);
        }
        else {
            update(EPOLL_CTL_MOD, channel);
        }

    }
}
#if 0
void EpollPoller::addChannel(Channel *channel) {
    /** 将channel 添加到 channels */
    int fd = channel->getFd();
    assert(channels_.find(fd) == channels_.end());
    channels_[fd] = channel;
    update(EPOLL_CTL_ADD, channel);
}
void EpollPoller::modifyChannel(Channel *channel) {
    update(EPOLL_CTL_MOD, channel);
}
#endif

void EpollPoller::removeChannel(Channel *channel) {
    /** 在确定channels_中有channel的前提下，删除channel */
    int fd = channel->getFd();
    STREAM_TRACE << "fd = " << fd;
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(channel->isNoneEvent());
    int index = channel->getIndex();
    assert(index == kAdded || index == kDeleted);

    size_t n = channels_.erase(fd);
    (void)n;
    assert(n == 1);
    /** kDeleted之前已经从epoll中删除了，而kAdded还没有 */
    if (index == kAdded) {
        update(EPOLL_CTL_DEL, channel);
    }
    /** 设置 index未添加的 */
    channel->setIndex(kNew);
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

