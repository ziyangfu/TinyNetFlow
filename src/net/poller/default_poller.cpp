//
// Created by fzy on 23-3-13.
//
/*! 根据环境变量 MUDUO_USE_POLL 选择后端 */
#include "../poller.h"
#include "poll_poller.h"
#include "epoll_poller.h"

#include <stdlib.h>

using namespace muduo::net;

Poller* Poller::newDefaultPoller(muduo::net::EventLoop *loop) {
    if (getenv("MUDUO_USE_POLL")) {
        return new PollPoller(loop);
    }
    else {
        return new EpollPoller(loop);
    }
}