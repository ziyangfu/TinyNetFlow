//
// Created by fzy on 23-5-8.
//

#include "Channel.h"
#include "EventLoop.h"
#include <sys/epoll.h>

using namespace netflow;
using namespace netflow::net;

Channel::Channel(EventLoop* loop, int fd)
    : loop_(loop),
      fd_(fd),
      events_(0),
      activeEvents_(0)
{

}
Channel::~Channel() {

}
/*!
 * \brief 事件处理
 * \public */
void Channel::handleEvent(netflow::base::Timestamp receiveTime) {
    handleEventCallback(receiveTime);
}
/*!
 * \brief 根据epoll事件结果执行四种回调函数
 * \private */
void Channel::handleEventCallback(netflow::base::Timestamp receiveTime) {
    if ((activeEvents_ & EPOLLHUP) && !(activeEvents_ & EPOLLIN)) {
        if (closeCallback_) {
            closeCallback_();
        }
    }
    if (activeEvents_ & (EPOLLERR)) {
        if (errorCallback_) {
            errorCallback_();
        }
    }
    if (activeEvents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
        if (readCallback_) {
            readCallback_(receiveTime);
        }
    }
    if (activeEvents_ & (EPOLLOUT)) {
        if (writeCallback_) {
            writeCallback_();
        }
    }
}

void Channel::addChannel() {
    loop_->addChannel(this);
}

void Channel::removeChannel() {
    loop_->removeChannel(this);
}

void Channel::modifyChannel() {
    loop_->modifyChannel(this);
}
