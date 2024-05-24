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
 *      供 TCP server 使用的 TcpServerSocket接口，是TcpSocket的适配器
 *      RAII类，持有 socket fd
 * \file
 *      TcpServerSocket.cpp
 * ----------------------------------------------------------------------------------------- */

#include "IO/net/TcpServerSocket.h"
#include "IO/net/TcpSocket.h"

#include <cstring>

using namespace osadaptor::net;

TcpServerSocket::~TcpServerSocket() {
    tcpSocket::close(sockfd_);
}

void TcpServerSocket::bindAddr(const InetAddr& localAddr) {
    tcpSocket::bind(sockfd_, localAddr.getSockAddr());
}

void TcpServerSocket::listen() {
    tcpSocket::listen(sockfd_);
}

int TcpServerSocket::accept(InetAddr* peerAddr) {
    struct sockaddr_in6 addr;
    memset(&addr, 0, sizeof addr);
    int connfd = tcpSocket::accept(sockfd_, &addr);
    if (connfd >= 0)
    {
        peerAddr->setSockAddrInet6(addr);
    }
    return connfd;
}

void TcpServerSocket::shutdownWrite() {
    tcpSocket::shutdownWrite(sockfd_);
}

/** set sock options */
void TcpServerSocket::setTcpNoDelay(bool on) {
    tcpSocket::setTcpNoDelay(sockfd_, on);
}

void TcpServerSocket::setReuseAddr(bool on) {
    tcpSocket::setReuseAddr(sockfd_, on);
}

void TcpServerSocket::setReusePort(bool on) {
    tcpSocket::setReusePort(sockfd_, on);
}

void TcpServerSocket::setKeepAlive(bool on) {
    tcpSocket::setKeepAlive(sockfd_, on);
}

void TcpServerSocket::setPriority(int32_t priority) {
    tcpSocket::setPriority(sockfd_, priority);
}

void TcpServerSocket::setTTL(uint8_t hops) {
    tcpSocket::setUnicastTtl(sockfd_, hops);
}

bool TcpServerSocket::getTcpInfo() const {
    return true;
}

