/** ----------------------------------------------------------------------------------------
 * \copyright
 * Copyright (c) 2023 by the TinyNetFlow project authors. All Rights Reserved.
 *
 * This file is open source software, licensed to you under the ter；ms
 * of the Apache License, Version 2.0 (the "License").  See the NOTICE file
 * distributed with this work for additional information regarding copyright
 * ownership.  You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 * -----------------------------------------------------------------------------------------
 * \brief
 *      封装 Linux epoll
 * \file
 *      EpollPoller.cpp
 * ----------------------------------------------------------------------------------------- */

#include "IO/reactor/EpollPoller.h"
#include "IO/reactor/Channel.h"
#include "IO/reactor/EventLoop.h"

#include <spdlog/spdlog.h>
#include <sys/epoll.h>
#include <strings.h>

using namespace osadaptor::net;
using namespace osadaptor::time;

EpollPoller::EpollPoller(EventLoop* loop)
    : epollFd_(::epoll_create1(EPOLL_CLOEXEC)),
      events_(kInitEventListSize),
      ownerLoop_(loop)
{
}

EpollPoller::~EpollPoller() {
    ::close(epollFd_);
}

Timestamp EpollPoller::poll(int timeoutMs, ChannelLists* activeChannels) {
    SPDLOG_TRACE("fd total count {}", channels_.size());
    int numActiveEvents = ::epoll_wait(epollFd_, &(*events_.begin()),
                                       static_cast<int>(events_.size()),
                                       timeoutMs);
    Timestamp now(Timestamp::now());
    if(numActiveEvents > 0) {
        SPDLOG_TRACE("{} events happened", numActiveEvents);
        //! 将活动的channel push到 activeChannels容器
        fillActiveChannel(numActiveEvents, activeChannels);
        //! 2倍扩容， 目前缺陷，只能扩容，不能缩
        if (static_cast<size_t>(numActiveEvents) == events_.size()) {
            events_.resize(events_.size() * 2);
        }
    }
    else if (numActiveEvents == 0) {
        /** 超时， 无事发生 */
        SPDLOG_TRACE("nothing happened");
    }
    else {
        SPDLOG_ERROR("epoll::poll() error");

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
/*!
 * \brief 将channel添加至epoll
 * */
void EpollPoller::updateChannel(Channel *channel) {
    assertInLoopThread();
    Channel::ChannelStatus index = channel->getIndex();
    int fd = channel->getFd();
    SPDLOG_TRACE("fd is {}, events is {}, index is {}", fd, channel->getEvents(), index);
    /** 未添加的，或者已经删除的，有添加的需求 */
    if (index == Channel::ChannelStatus::kNew || index == Channel::ChannelStatus::kDeleted) {
        if (index == Channel::ChannelStatus::kNew) {
            assert(channels_.find(fd) == channels_.end());
            channels_[fd] = channel;
        }
        else {
            /** epoll中已经删除了，但 channels中还有，需要重新添加到epoll中 */
            assert(channels_.find(fd) != channels_.end());
            assert(channels_[fd] == channel);
        }
        channel->setIndex(Channel::ChannelStatus::kAdded);
        update(EPOLL_CTL_ADD, channel);
    }
    /** 已经添加的，有修改的需求 */
    else {
        /** 一个类型为 void 的表达式，技巧，为了避免变量未被使用的编译器警告。因为如果变量 fd 没有被使用，某些编译器会发出警告 */
        (void)fd;
        assert(channels_.find(fd) != channels_.end());
        assert(channels_[fd] == channel);
        assert(index == Channel::ChannelStatus::kAdded);
        if (channel->isNoneEvent()) {
            /** 若channel关闭读，关闭写，那么将channel从epoll中移除，只保留在channels_ */
            update(EPOLL_CTL_DEL, channel);
            channel->setIndex(Channel::ChannelStatus::kDeleted);
        }
        else {
            update(EPOLL_CTL_MOD, channel);
        }

    }
}

void EpollPoller::removeChannel(Channel *channel) {
    /** 在确定channels_中有channel的前提下，删除channel */
    int fd = channel->getFd();
    SPDLOG_TRACE("fd is {}", fd);
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(channel->isNoneEvent());
    Channel::ChannelStatus index = channel->getIndex();
    assert(index == Channel::ChannelStatus::kAdded || index == Channel::ChannelStatus::kDeleted);

    size_t n = channels_.erase(fd);
    (void)n;
    assert(n == 1);
    /** kDeleted之前已经从epoll中删除了，而kAdded还没有 */
    if (index == Channel::ChannelStatus::kAdded) {
        update(EPOLL_CTL_DEL, channel);
    }
    /** 设置 index未添加的 */
    channel->setIndex(Channel::ChannelStatus::kNew);
}

void EpollPoller::update(int operation, Channel *channel) {
    struct epoll_event event;
    bzero(&event, sizeof event);
    event.events = channel->getEvents();
    event.data.ptr = channel;
    int fd = channel->getFd();
    SPDLOG_TRACE("Epoll control operation = {}, fd = {}", operationToString(operation), fd);
    if (::epoll_ctl(epollFd_, operation, fd, &event) < 0) {
        SPDLOG_ERROR("Epoll control operation = {}, fd = {}", operationToString(operation), fd);
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

bool EpollPoller::hasChannel(Channel *channel) {
    assertInLoopThread();
    int fd = channel->getFd();
    auto it = channels_.find(fd);
    return it !=channels_.end() && it->second == channel;
}

