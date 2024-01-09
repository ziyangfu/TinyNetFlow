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
 *      用于TCP建立连接后的数据发送与接收，供 TCP客户端与服务端用
 * \file
 *      TcpConnection.h
 * ----------------------------------------------------------------------------------------- */

#ifndef TINYNETFLOW_OSADAPTOR_TCPCONNECTION_H
#define TINYNETFLOW_OSADAPTOR_TCPCONNECTION_H

#include <memory>
#include <any>
#include <string>
#include <atomic>

#include "IO/net/InetAddr.h"
#include "IO/net/Callbacks.h"
#include "IO/net/Buffer.h"
#include "time/Timestamp.h"

namespace netflow::osadaptor::net {

class Channel;
class EventLoop;
class TcpServerSocket;

class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
private:
    enum class StateE : uint8_t {
        kDisconnected,
        kConnecting,
        kConnected,
        kDisconnecting
    };

    //EventLoop* loop_;
    std::shared_ptr<EventLoop> loop_;
    const std::string name_;
    std::atomic<StateE> state_;
    bool reading;

    /** 以下类不暴露给客户端 */
    std::unique_ptr<TcpServerSocket> socket_;
    std::unique_ptr<Channel> channel_;
    const InetAddr localAddr_;
    const InetAddr peerAddr_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    HighWaterMarkCallback highWaterMarkCallback_;
    CloseCallback closeCallback_;

    size_t highWaterMark_;
    //std::unique_ptr<Buffer> inputBuffer_;
    //std::unique_ptr<Buffer> outputBuffer_;
    Buffer inputBuffer_;
    Buffer outputBuffer_;
    std::any context_;

public:
    // TcpConnection(EventLoop* loop, const std::string& name, int sockfd,
    //              const InetAddr& localAddr, const InetAddr& peerAddr);

    TcpConnection(std::shared_ptr<EventLoop>& loop, const std::string& name, int sockfd,
                  const InetAddr& localAddr, const InetAddr& peerAddr);


    ~TcpConnection();

    EventLoop* getLoop() const { return loop_; }
    const std::string& getName() const { return name_; }
    const InetAddr& getLocalAddr() const { return localAddr_; }
    const InetAddr& getPeerAddr() const { return peerAddr_; }

    bool isConnected() const { return state_ == StateE::kConnected; }
    bool isDisconnected() const { return state_ == StateE::kDisconnected; }

    void send(const void* message, int len);
    void send(const std::string& message);
    void send(Buffer* buf);
    void shutdown();
    void forceClose();
    void forceCloseWithDelay(double seconds);
    void setTcpNoDelay(bool on);

    void startRead();
    void stopRead();
    bool isReading() const { return reading; }

    void setContext(const std::any& context) { context_ = context; }
    const std::any& getContext() const { return context_; }
    std::any* getMutableContext() { return &context_; }

    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback& cb) { writeCompleteCallback_ = cb; }
    void setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark) {
        highWaterMarkCallback_ = cb;
        highWaterMark_ = highWaterMark;
    }

    Buffer* getInputBufferPtr() { return &inputBuffer_; }
    Buffer* getOutputBufferPtr() { return &outputBuffer_; }
    /*!
     * \private 内部使用 */
    void setCloseCallback(const CloseCallback& cb) { closeCallback_ = cb; }
    /*!
     * \brief 供 TcpServer 接收新连接时使用 */
    void connectEstablished();
    void connectDestroyed();

private:
    void handleRead(time::Timestamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();

    void sendInLoop(const void* message, size_t len);
    void sendInLoop(const std::string& message);

    void shutdownInLoop();
    void forceCloseInLoop();
    void setState(StateE s)  { state_ = s; }

    const char* stateToString() const;
    void startReadInLoop();
    void stopReadInLoop();
};

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

} // namespace netflow::osadaptor::net



#endif //TINYNETFLOW_OSADAPTOR_TCPCONNECTION_H
