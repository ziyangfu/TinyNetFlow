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
 *      供 TCP server 使用的 socket接口，是TcpSocket的适配器
 * \file
 *      TcpServerSocket.h
 * ----------------------------------------------------------------------------------------- */

#ifndef TINYNETFLOW_OSADAPTOR_SOCKET_H
#define TINYNETFLOW_OSADAPTOR_SOCKET_H

#include <arpa/inet.h>
#include "IO/net/InetAddr.h"

namespace netflow::osadaptor::net {

class TcpServerSocket {
private:
    const int sockfd_;
public:
    explicit TcpServerSocket(int sockfd)
        :sockfd_(sockfd)
    {
    }
    TcpServerSocket(const TcpServerSocket& other) = delete;
    TcpServerSocket& operator=(const TcpServerSocket& other) = delete;
    TcpServerSocket(TcpServerSocket&& other) = delete;
    TcpServerSocket& operator=(TcpServerSocket&& other) = delete;
    ~TcpServerSocket();

    int getFd() const { return sockfd_; }
    void bindAddr(const InetAddr& localAddr);
    void listen();
    int accept(InetAddr* peerAddr);
    void shutdownWrite();

    /** set socket option */
    void setTcpNoDelay(bool on);
    void setReuseAddr(bool on);
    void setReusePort(bool on);
    void setKeepAlive(bool on);
    void setPriority(int32_t priority);
    void setTTL(uint8_t hops);
    /** for debug */
    bool getTcpInfo() const;
};

} // namespace netflow::osadaptor::net



#endif //TINYNETFLOW_OSADAPTOR_SOCKET_H
