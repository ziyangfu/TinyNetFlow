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
 *      TCP 服务端
 * \file
 *      TcpServer.cpp
 * ----------------------------------------------------------------------------------------- */

#include "IO/net/TcpServer.h"
#include "IO/net/Acceptor.h"
#include "IO/net/TcpSocket.h"
#include "IO/reactor/EventLoop.h"
#include "IO/reactor/EventLoopThreadPool.h"

using namespace netflow::osadaptor::net;

TcpServer::TcpServer(EventLoop *loop, const InetAddr &listenAddr,
                     const std::string &name, TcpServer::Option option)
    : loop_(loop),
      ipPort_(listenAddr.toStringIpPort()),
      name_(name),
      acceptor_(std::make_unique<Acceptor>(loop_, listenAddr, option == Option::kReusePort)),
      threadPool_(std::make_shared<EventLoopThreadPool>(loop_, name_)),
      nextConnId_(1),
      started_(false),
      connectionCallback_(defaultConnectionCallback),
      messageCallback_(defaultMessageCallback)
{
    acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this,
                                                  std::placeholders::_1, std::placeholders::_2));
}

TcpServer::TcpServer(std::shared_ptr<EventLoop>& loop, const InetAddr &listenAddr,
                     const std::string &name, TcpServer::Option option)
    : loop_(loop)
{

}









TcpServer::~TcpServer() {
    loop_->assertInLoopThread();
    for (auto& item : connections_) {
        TcpConnectionPtr conn{item.second};
        item.second.reset();
        conn->getLoop()->runInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
    }
}

void TcpServer::setThreadNum(int numThreads) {
    assert(numThreads >= 0);
    threadPool_->setThreadNum(numThreads);
}

void TcpServer::start() {
    if (!started_) {
        threadPool_->start(threadInitCallback_);
        /** 此时还没有开始listen模式 */
        assert(!acceptor_->listening());
        loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
    }
}

/*!
 * \private ***************************************************************************************/
void TcpServer::newConnection(int sockfd, const InetAddr &peerAddr) {
    loop_->assertInLoopThread();
    EventLoop* ioLoop = threadPool_->getNextLoop();
    char buf[64];
    snprintf(buf, sizeof buf, "-%s#%d", ipPort_.c_str(), nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;
    InetAddr localAddr{tcpSocket::getLocalAddr(sockfd)};
    TcpConnectionPtr conn{std::make_shared<TcpConnection>(ioLoop, connName, sockfd,
                                                          localAddr, peerAddr)};
    connections_[connName] = conn;
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
    /** TODO highWaterMarkCallback??? */
    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));

}

void TcpServer::removeConnection(const TcpConnectionPtr &conn) {
    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn) {
    loop_->assertInLoopThread();
    size_t  n = connections_.erase(conn->getName());
    (void) n;
    assert(static_cast<int>(n) == 1);  /** 清除了1个 */
    EventLoop* ioLoop = conn->getLoop();
    ioLoop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}