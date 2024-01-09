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
 *      连接器，供TCP客户端或UDS客户端使用
 * \file
 *      Connector.cpp
 * ----------------------------------------------------------------------------------------- */

#include "IO/net/Connector.h"
#include "IO/reactor/Channel.h"
#include "IO/reactor/EventLoop.h"
#include "IO/net/TcpSocket.h"

#include <spdlog/spdlog.h>
#include <cerrno>
#include <cassert>
#include <cstring>

using namespace netflow::osadaptor::net;

const int Connector::kMaxRetryDelayMs;

#if 0
Connector::Connector(EventLoop *loop, const InetAddr &serverAddr)
    : loop_(loop),
      serverAddr_(serverAddr),
      connect_(false),
      state_(States::kDisconnected),
      retryDelayMs_(kInitRetryDelayMs)
{
}
#endif

Connector::Connector(std::shared_ptr<EventLoop> &loop, const InetAddr &serverAddr)
    : loop_(loop),
      serverAddr_(serverAddr),
      connect_(false),
      state_(States::kDisconnected),
      retryDelayMs_(kInitRetryDelayMs)
{

}

Connector::~Connector() {

}

/*!
 * \brief 启动连接器
 * \details 此时调用runInLoop的线程不是loop线程，因此需要进行唤醒， startInLoop实际在doPendingFunctors中执行
 * */
void Connector::start() {
    connect_ = true;
    loop_->runInLoop(std::bind(&Connector::startInLoop, this));
}

void Connector::startInLoop() {
    loop_->assertInLoopThread();
    assert(state_ == States::kDisconnected);
    if (connect_)
    {
        connect();
    }
    else
    {
        SPDLOG_DEBUG("do not connect");
    }
}

void Connector::stop()
{
    connect_ = false;
    loop_->queueInLoop(std::bind(&Connector::stopInLoop, this)); // FIXME: unsafe
    // FIXME: cancel timer
}

void Connector::stopInLoop()
{
    loop_->assertInLoopThread();
    if (state_ == States::kConnecting)
    {
        setState(States::kDisconnected);
        int sockfd = removeAndResetChannel();
        retry(sockfd);
    }
}

void Connector::connect()
{
    int sockfd = tcpSocket::createNonblockingSocket(serverAddr_.getInetFamily());  // 创建 socket
    int ret = tcpSocket::connect(sockfd, serverAddr_.getSockAddr());   // 建立连接
    int savedErrno = (ret == 0) ? 0 : errno;
    /** 当前连接已经建立成功、正在进行中，或者被信号中断 */
    switch (savedErrno)
    {
        case 0:  // 建立成功
        case EINPROGRESS:  // TCP 三次握手仍在继续
        case EINTR:
        case EISCONN:
            connecting(sockfd);  // 处理连接结果
            break;

        case EAGAIN:  // 在非阻塞模式下，连接不可立即建立，需要重试
        case EADDRINUSE:
        case EADDRNOTAVAIL:
        case ECONNREFUSED:
        case ENETUNREACH:
            retry(sockfd); // 重试连接
            break;

        case EACCES: // 其他错误码表示连接失败，关闭套接字并记录错误日志
        case EPERM:
        case EAFNOSUPPORT:
        case EALREADY:
        case EBADF:
        case EFAULT:
        case ENOTSOCK:
            SPDLOG_ERROR("connect error in Connector::startInLoop {}", savedErrno);
            tcpSocket::close(sockfd);
            break;

        default:
            SPDLOG_ERROR("Unexpected error in Connector::startInLoop {}", savedErrno);
            tcpSocket::close(sockfd);
            // connectErrorCallback_();
            break;
    }
}

void Connector::restart()
{
    loop_->assertInLoopThread();
    setState(States::kDisconnected);
    retryDelayMs_ = kInitRetryDelayMs;
    connect_ = true;
    startInLoop();
}

void Connector::connecting(int sockfd)   // socket连接建立后，处理上层
{
    setState(States::kConnecting);
    assert(!channel_);
    channel_.reset(new Channel(loop_.get(), sockfd));
    channel_->setWriteCallback(  // 建立TCP连接阶段时的写回调
            std::bind(&Connector::handleWrite, this)); // FIXME: unsafe
    channel_->setErrorCallback(
            std::bind(&Connector::handleError, this)); // FIXME: unsafe

    // channel_->tie(shared_from_this()); is not working,
    // as channel_ is not managed by shared_ptr
    channel_->enableWriting();   // 最终会把channel添加到epoll中
}

int Connector::removeAndResetChannel()
{
    channel_->disableAll();  // 清除事件
    channel_->removeChannel();   // 从 epoll中移除 channel
    int sockfd = channel_->getFd();  // 保存fd
    // Can't reset channel_ here, because we are inside Channel::handleEvent
    loop_->queueInLoop(std::bind(&Connector::resetChannel, this)); // FIXME: unsafe
    return sockfd;
}

void Connector::resetChannel()
{
    channel_.reset();  /** 释放当前指针所拥有的对象，并将channel_置为nullptr */
}

/*!
 * \details
 *      removeAndResetChannel: 为什么要移除重置？？ 因为连接阶段结束，这些事件不需要了，而上层还需要sockfd
 *      获取套接字上待处理的错误数量，实际作用是再次确认是否成功建立连接
 * */
void Connector::handleWrite()
{
    SPDLOG_TRACE("Connector::handleWrite {}", state_);
    if (state_ == States::kConnecting)
    {
        int sockfd = removeAndResetChannel();
        int err = tcpSocket::getSocketError(sockfd);
        if (err)
        {
            SPDLOG_WARN("Connector::handleWrite - SO_ERROR ={}", err);
            retry(sockfd);
        }
        else if (tcpSocket::isSelfConnect(sockfd))
        {
            SPDLOG_WARN("Connector::handleWrite - Self connect");
            retry(sockfd);
        }
        else
        {
            setState(States::kConnected);
            if (connect_)
            {
                newConnectionCallback_(sockfd);   /** 执行 TcpClient中的 newConnection() */
            }
            else
            {
                tcpSocket::close(sockfd);
            }
        }
    }
    else
    {
        // what happened?
        assert(state_ == States::kDisconnected);
    }
}

void Connector::handleError()
{
    if (state_ == States::kConnecting)
    {
        int sockfd = removeAndResetChannel();
        int err = tcpSocket::getSocketError(sockfd);
        retry(sockfd);
    }
}

void Connector::retry(int sockfd)
{
    tcpSocket::close(sockfd);
    setState(States::kDisconnected);
    if (connect_)
    {
        SPDLOG_INFO("Retry connecting to {} in {} ms", serverAddr_.toStringIpPort(),retryDelayMs_);

        loop_->runAfter(retryDelayMs_/1000.0,
                        std::bind(&Connector::startInLoop, shared_from_this()));
        retryDelayMs_ = std::min(retryDelayMs_ * 2, kMaxRetryDelayMs);
    }
    else
    {
        SPDLOG_DEBUG("do not connect");
    }
}