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
 *      TCP 服务端, 支持单IO线程模型与多IO线程模型
 * \file
 *      TcpServer.h
 * ----------------------------------------------------------------------------------------- */

#ifndef TINYNETFLOW_OSADAPTOR_TCPSERVER_H
#define TINYNETFLOW_OSADAPTOR_TCPSERVER_H

#include "IO/net/TcpConnection.h"
#include "IO/net/Callbacks.h"

#include <map>
#include <functional>
#include <memory>
#include <atomic>

namespace netflow::osadaptor::net {

class Acceptor;
class EventLoop;
class EventLoopThreadPool;

class TcpServer {
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;
    enum class Option : uint8_t {
        kNoReusePort,
        kReusePort
    };

private:
    using ConnectionMap = std::map<std::string, TcpConnectionPtr>;

    EventLoop* loop_;
    const std::string ipPort_;
    const std::string name_;
    std::unique_ptr<Acceptor> acceptor_;   /** Acceptor 独属于 TcpServer */
    std::unique_ptr<EventLoopThreadPool> threadPool_;

    std::atomic_bool started_;
    int nextConnId_;
    ConnectionMap connections_;   /** 存储所有连接 */

    ConnectionCallback      connectionCallback_;
    MessageCallback         messageCallback_;
    WriteCompleteCallback   writeCompleteCallback_;
    ThreadInitCallback      threadInitCallback_;

public:
    TcpServer(EventLoop* loop, const InetAddr& listenAddr,
              const std::string& name, Option option = Option::kNoReusePort);
    ~TcpServer();
    const std::string& getIpPort() const { return ipPort_; }
    const std::string& getName() const { return name_; }
    EventLoop* getLoop() const { return loop_; }

    void setThreadNum(int numThreads);
    void setThreadInitCallback(const ThreadInitCallback& cb) { threadInitCallback_ = cb; }
    EventLoopThreadPool* getThreadPoolPtr() { return threadPool_.get(); }

    void start();

    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback& cb) { writeCompleteCallback_ = cb; }

private:
    void newConnection(int sockfd, const InetAddr& peerAddr);
    void removeConnection(const TcpConnectionPtr& conn);
    void removeConnectionInLoop(const TcpConnectionPtr& conn);

};

} // namespace netflow::osadaptor::net



#endif //TINYNETFLOW_OSADAPTOR_TCPSERVER_H
