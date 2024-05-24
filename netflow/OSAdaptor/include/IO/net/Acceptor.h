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
 *      Acceptor.h
 * ----------------------------------------------------------------------------------------- */

#ifndef TINYNETFLOW_OSADAPTOR_ACCEPTOR_H
#define TINYNETFLOW_OSADAPTOR_ACCEPTOR_H

#include "IO/reactor/Channel.h"
#include "IO/net/TcpServerSocket.h"

namespace osadaptor::net {

class EventLoop;
class InetAddr;

class Acceptor {
public:
    using NewConnectionCallback = std::function<void (int sockfd, const InetAddr&)>;
private:
    EventLoop* loop_;
    TcpServerSocket acceptSocket_;
    Channel acceptChannel_;
    bool listening_;
    int idleFd_;

    NewConnectionCallback newConnectionCallback_;
public:
    /** loop 不一定需要，目前只用来确认，acceptor的所有函数是在loop线程上运行 */
    Acceptor(EventLoop* loop, const InetAddr& listenAddr, bool reuseport);

    ~Acceptor();
    void setNewConnectionCallback(const NewConnectionCallback& cb)
    { newConnectionCallback_ = cb; }

    void listen();

    bool listening() const { return listening_; }

private:
    void handleRead();
};
} // namespace osadaptor::net

#endif //TINYNETFLOW_OSADAPTOR_ACCEPTOR_H
