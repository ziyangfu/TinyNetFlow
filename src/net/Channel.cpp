//
// Created by fzy on 23-5-8.
//

#include "Channel.h"
#include "EventLoop.h"
#include "../base/Logging.h"

#include <sys/epoll.h>
#include <assert.h>


using namespace netflow::base;
using namespace netflow::net;

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;

Channel::Channel(EventLoop* loop, int fd)
    : loop_(loop),
      fd_(fd),
      events_(0),
      activeEvents_(0),
      tied_(false),
      eventHandling_(false),
      addedToLoop_(false),
      index_(-1)  /** 默认是 kNew */
{

}
Channel::~Channel() {
    assert(!eventHandling_);
    assert(!addedToLoop_);
    if (loop_->isInLoopThread())
    {
        assert(!loop_->hasChannel(this));
    }
}
/*!
 * \brief 事件处理
 * TODO： 为什么要判断 tied_，但后续操作后一致，调用 handleEventCallback
 * \public */
void Channel::handleEvent(netflow::base::Timestamp receiveTime) {
    std::shared_ptr<void> guard;
    if (tied_)  /** TCP 连接还在 */
    {
        guard = tie_.lock();
        if (guard)
        {
            handleEventCallback(receiveTime);
        }
    }
    else
    {
        handleEventCallback(receiveTime);
    }


}
/*!
 * \brief 根据epoll事件结果执行四种回调函数
 * \private */
void Channel::handleEventCallback(netflow::base::Timestamp receiveTime) {
    eventHandling_ = true;
    LOG_TRACE(" activeEvents_ = {}", activeEvents_);
    /** 关闭事件回调 */
    if ((activeEvents_ & EPOLLHUP) && !(activeEvents_ & EPOLLIN)) {
        if (closeCallback_) {
            LOG_TRACE("close event callback");
            closeCallback_();
        }
    }
    /** 错误事件回调 */
    if (activeEvents_ & (EPOLLERR)) {
        if (errorCallback_) {
            LOG_TRACE("error event callback");
            errorCallback_();
        }
    }
    /** 读事件回调 */
    if (activeEvents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
        if (readCallback_) {
            LOG_TRACE("read event callback");
            readCallback_(receiveTime);
        }
    }
    /** 写事件回调 */
    if (activeEvents_ & (EPOLLOUT)) {
        if (writeCallback_) {
            LOG_TRACE("write event callback");
            writeCallback_();
        }
    }
    eventHandling_ = false;
}
/*!
 * \brief  判断TcpConnection是否还存在 */
void Channel::tie(const std::shared_ptr<void> & obj) {
    /**  将 weak_ptr 提升为 shared_ptr， 可以用来判断 TcpConnection 还存在*/
    tie_ = obj;
    tied_ = true;
}

void Channel::update() {
    addedToLoop_ = true;
    loop_->updateChannel(this);
}

void Channel::removeChannel() {
    assert(isNoneEvent());
    addedToLoop_ = false;
    loop_->removeChannel(this);
}