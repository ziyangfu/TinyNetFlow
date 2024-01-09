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
 *      Connector.h
 * ----------------------------------------------------------------------------------------- */

#ifndef TINYNETFLOW_OSADAPTOR_CONNECTOR_H
#define TINYNETFLOW_OSADAPTOR_CONNECTOR_H

#include "IO/net/InetAddr.h"

#include <functional>
#include <memory>
#include <atomic>

namespace netflow::osadaptor::net {

class Channel;
class EventLoop;
/** this指针由智能指针管理 */
class Connector : public std::enable_shared_from_this<Connector> {
public:
    using NewConnectionCallback = std::function<void (int sockfd)>;
private:
    enum class States : uint8_t {
        kDisconnected,
        kConnecting,
        kConnected
    };
    //EventLoop* loop_;
    std::shared_ptr<EventLoop> loop_;
    InetAddr serverAddr_;
    std::atomic_bool connect_;
    std::atomic<States> state_;
    std::unique_ptr<Channel> channel_;
    NewConnectionCallback newConnectionCallback_;
    int retryDelayMs_;

    static const int kMaxRetryDelayMs = 30*1000;
    static const int kInitRetryDelayMs = 500;
public:
    //Connector(EventLoop* loop, const InetAddr& serverAddr);
    Connector(std::shared_ptr<EventLoop>& loop, const InetAddr& serverAddr);
    ~Connector();

    void setNewConnectionCallback(const NewConnectionCallback& cb)
    { newConnectionCallback_ = cb; }

    void start();  // can be called in any thread
    void restart();  // must be called in loop thread
    void stop();  // can be called in any thread

private:
    void setState(States s) { state_ = s; }
    void startInLoop();
    void stopInLoop();
    void connect();
    void connecting(int sockfd);
    void handleWrite();
    void handleError();
    void retry(int sockfd);
    int removeAndResetChannel();
    void resetChannel();
};
} // namespace netflow::osadaptor::net



#endif //TINYNETFLOW_OSADAPTOR_CONNECTOR_H
