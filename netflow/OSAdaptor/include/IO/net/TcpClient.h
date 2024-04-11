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
 *      TCP客户端
 * \file
 *      TcpClient.h
 * ----------------------------------------------------------------------------------------- */

#ifndef TINYNETFLOW_OSADAPTOR_TCPCLIENT_H
#define TINYNETFLOW_OSADAPTOR_TCPCLIENT_H

#include <memory>
#include <mutex>
#include "IO/net/TcpConnection.h"

namespace netflow::osadaptor::net {

class Connector;

class TcpClient {
private:
    EventLoop* loop_;
    std::unique_ptr<Connector> connector_;
    const std::string name_;
    std::atomic_bool retry_;
    std::atomic_bool connect_;
    int nextConnId_;
    std::mutex mutex_;
    TcpConnectionPtr connection_;

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;

public:
    TcpClient(EventLoop* loop, const InetAddr& serverAddr, const std::string& name);
    ~TcpClient();

    void connect();
    void disconnect();
    void stop();

    TcpConnectionPtr getTcpConnectionPtr();
    bool retry() const { return retry_; }
    void enableRetry() { retry_ = true; }

    const std::string& getName() const { return name_; }
    /* FIXME: 为什么之前在测试程序中，传引用会有问题 */
    void setConnectionCallback(ConnectionCallback cb) { connectionCallback_ = std::move(cb); }
    void setMessageCallback(MessageCallback cb) { messageCallback_ = std::move(cb); }
    void setWriteCompleteCallback(WriteCompleteCallback cb) { writeCompleteCallback_ = std::move(cb); }

private:
    void newConnection(int sockfd);
    void removeConnection(const TcpConnectionPtr& conn);


};
}  // namespace netflow::osadaptor::net



#endif //TINYNETFLOW_OSADAPTOR_TCPCLIENT_H
