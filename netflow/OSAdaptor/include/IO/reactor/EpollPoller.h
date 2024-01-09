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
 *      EpollPoller.h
 * ----------------------------------------------------------------------------------------- */

#ifndef TINYNETFLOW_OSADAPTOR_EPOLLPOLLER_H
#define TINYNETFLOW_OSADAPTOR_EPOLLPOLLER_H

#include <vector>
#include <map>
#include <sys/epoll.h>

#include "time/Timestamp.h"

namespace netflow::osadaptor::net {

class Channel; /** 前置声明 */
class EventLoop;

class EpollPoller {
public:
    using ChannelLists = std::vector<Channel*>;
private:
    static const int kInitEventListSize = 16;
    int epollFd_;
    std::map<int, Channel*> channels_;
    std::vector<struct epoll_event> events_;  //! 自适应
    EventLoop* ownerLoop_;

public:
    EpollPoller(EventLoop* loop);
    ~EpollPoller();
    time::Timestamp poll(int timeoutMs, ChannelLists* activeChannels);

    void updateChannel(Channel* channel); /** 将添加与修改合二为一，使得上层无需关心 */
    void removeChannel(Channel* channel);

    bool hasChannel(Channel* channel);
    void assertInLoopThread();
private:
    void update(int operation, Channel* channel);
    void fillActiveChannel(int numEvents, ChannelLists* activeChannels) const;
    static const char* operationToString(int op);
};

} // namespace netflow::osadaptor::net



#endif //TINYNETFLOW_OSADAPTOR_EPOLLPOLLER_H
