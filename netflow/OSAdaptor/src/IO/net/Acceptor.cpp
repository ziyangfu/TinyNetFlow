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
 *      接收器，供TCP服务端端或UDS服务端使用
 * \file
 *      Acceptor.cpp
 * ----------------------------------------------------------------------------------------- */

#include "IO/net/Acceptor.h"
#include "IO/net/InetAddr.h"
#include "IO/reactor/EventLoop.h"
#include "IO/net/TcpSocket.h"

#include <spdlog/spdlog.h>
#include <cerrno>
#include <cassert>
#include <string>
#include <unistd.h>
#include <fcntl.h>


using namespace netflow::osadaptor::net;


Acceptor::Acceptor(EventLoop *loop, const InetAddr &listenAddr, bool reuseport)
    :loop_(loop),
     acceptSocket_(tcpSocket::createNonblockingSocket(listenAddr.getInetFamily())),
     acceptChannel_(loop, acceptSocket_.getFd()),
     listening_(false),
     idleFd_(::open("/dev/null", O_RDONLY | O_CLOEXEC)) /** 满连接后的处理方法 */
{
    assert(idleFd_ >= 0);
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(reuseport);
    acceptSocket_.bindAddr(listenAddr);
    acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

#if 0
Acceptor::Acceptor(std::shared_ptr<EventLoop> &loop, const InetAddr &listenAddr,
                   bool reUsePort)
        : loop_(loop),
          acceptSocket_(tcpSocket::createNonblockingSocket(listenAddr.getInetFamily())),
          acceptChannel_(std::make_unique<Channel>(loop.get(), acceptSocket_.getFd())),
          listening_(false),
          idleFd_(::open("/dev/null", O_RDONLY | O_CLOEXEC)) /** 满连接后的处理方法 */
{
}
#endif

Acceptor::~Acceptor() {
    acceptChannel_.disableAll();
    acceptChannel_.removeChannel();
    ::close(idleFd_);
}

void Acceptor::listen() {
    loop_->assertInLoopThread();
    listening_ = true;
    acceptSocket_.listen();          /** 开启监听模式 */
    acceptChannel_.enableReading();  /** 注册进epoll */
}

void Acceptor::handleRead() {
    loop_->assertInLoopThread();
    InetAddr peerAddr;  /** 接收客户端地址 */
    //FIXME loop until no more
    int connfd = acceptSocket_.accept(&peerAddr);
    if (connfd >= 0)
    {
        // std::string hostport = peerAddr.toStringIpPort();
        // SPDLOG_TRACE("Accepts of {}", hostport);
        if (newConnectionCallback_)
        {
            newConnectionCallback_(connfd, peerAddr);
        }
        else
        {
            tcpSocket::close(connfd);
        }
    }
    else
    {
        SPDLOG_ERROR("error in Acceptor::handleRead");
        // Read the section named "The special problem of
        // accept()ing when you can't" in libev's doc.
        // By Marc Lehmann, author of libev.
        if (errno == EMFILE)
        {
            /** 如果服务端的连接已经满了，则使用以下方法剔除新连接 */
            ::close(idleFd_);
            idleFd_ = ::accept(acceptSocket_.getFd(), nullptr, nullptr);
            ::close(idleFd_);
            idleFd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
    }
}
