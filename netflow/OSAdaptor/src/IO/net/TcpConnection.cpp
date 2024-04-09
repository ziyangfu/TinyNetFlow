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
 *      TcpConnection.cpp
 * ----------------------------------------------------------------------------------------- */

#include "IO/net/TcpConnection.h"

#include "IO/reactor/Channel.h"
#include "IO/reactor/EventLoop.h"
#include "IO/net/TcpServerSocket.h"
#include "IO/net/TcpSocket.h"

#include <cerrno>
#include <string_view>
#include <spdlog/spdlog.h>

using namespace netflow::osadaptor::net;
using namespace netflow::osadaptor::time;
/*!
 * \brief 全局函数，设置初始连接回调，什么事也不做
 * */
void defaultConnectionCallback(const TcpConnectionPtr& conn)
{
    SPDLOG_TRACE("{} -> {} is {}", conn->getLocalAddr().toStringIpPort(),
                                 conn->getPeerAddr().toStringIpPort(),
                                 conn->isConnected());
}
/*!
 * \brief 全局函数，设置初始消息回调， buffer的读写“指针”设置在初始点
 * */
void defaultMessageCallback(const TcpConnectionPtr&, Buffer* buf, Timestamp)  // TODO  Timestamp
{
    buf->retrieveAll();
}


TcpConnection::TcpConnection(EventLoop *loop, const std::string &name, int sockfd,
                             const InetAddr &localAddr, const InetAddr &peerAddr)
        : loop_(loop),
          name_(name),
          socket_(std::make_unique<TcpServerSocket>(sockfd)),
          channel_(std::make_unique<Channel>(loop_, sockfd)),
          localAddr_(localAddr),
          peerAddr_(peerAddr),
          state_(StateE::kConnecting),
          reading(true),
          highWaterMark_(64*1024*1024)
{
    /** 设置建立连接时的回调
     * bind绑定类成员函数时，第一个参数表示对象的成员函数的指针，第二个参数表示对象的地址
     * std::bind(callableFunc,_1,2)等价于std::bind (&callableFunc,_1,2) */
    channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
    channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));
    /** 设置 TCP 保活机制 */
    socket_->setKeepAlive(true);
}

#if 0
TcpConnection::TcpConnection(std::shared_ptr<EventLoop> &loop, const std::string &name, int sockfd,
                             const InetAddr &localAddr,
                             const InetAddr &peerAddr)
        : loop_(loop),
          name_(name),
          socket_(std::make_unique<TcpServerSocket>(sockfd)),
          channel_(std::make_unique<Channel>(loop_.get(), sockfd)),
          localAddr_(localAddr),
          peerAddr_(peerAddr),
          state_(StateE::kConnecting),
          reading(true),
          highWaterMark_(64*1024*1024)
{
    /** 设置建立连接时的回调
    * bind绑定类成员函数时，第一个参数表示对象的成员函数的指针，第二个参数表示对象的地址
    * std::bind(callableFunc,_1,2)等价于std::bind (&callableFunc,_1,2) */
    channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
    channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));
    /** 设置 TCP 保活机制 */
    socket_->setKeepAlive(true);
}
#endif


TcpConnection::~TcpConnection() {
    assert(state_ == StateE::kDisconnected);
}

void TcpConnection::send(const void *message, int len) {
    //
}

void TcpConnection::send(const std::string &message) {
    if (state_ == StateE::kConnected) {
        if (loop_->isInLoopThread()) {
            sendInLoop(message);
        }
        else {
            /** 获得 sendInLoop的函数指针 */
            void (TcpConnection::*fp)(const std::string& message) = &TcpConnection::sendInLoop;
            /** 唤醒后在IO线程中执行 */
            loop_->runInLoop(std::bind(fp, this, message));  /** TODO: string_view */
        }
    }
}

void TcpConnection::send(Buffer *buf) {
    if (state_ == StateE::kConnected) {
        if (loop_->isInLoopThread()) {
            sendInLoop(buf->peek(), buf->readableBytes());
            buf->retrieveAll();  /** buffer 中数据全部发出，读写指针退回初始状态 */
        }
        else {
            void (TcpConnection::*fp)(const std::string& message) = &TcpConnection::sendInLoop;
            loop_->runInLoop(std::bind(fp, this, buf->retrieveAllAsString()));
        }
    }
}

void TcpConnection::shutdown() {
    if (state_ == StateE::kConnected) {
        setState(StateE::kDisconnecting);
        loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
    }
}

void TcpConnection::forceClose() {
    if (state_ == StateE::kConnected || state_ == StateE::kDisconnecting) {
        setState(StateE::kDisconnecting);
        loop_->queueInLoop(std::bind(&TcpConnection::forceCloseInLoop, shared_from_this()));
    }
}

void TcpConnection::forceCloseWithDelay(double seconds) {

}

void TcpConnection::setTcpNoDelay(bool on) {
    socket_->setTcpNoDelay(on);
}

void TcpConnection::startRead() {
    loop_->runInLoop(std::bind(&TcpConnection::startReadInLoop, this));
}

void TcpConnection::stopRead() {
    loop_->runInLoop(std::bind(&TcpConnection::stopReadInLoop, this));
}

/*!
 * \brief TCP连接建立，并且所有上层回调函数都设置完毕后，告诉上层，连接已建立
 * */
void TcpConnection::connectEstablished() {
    loop_->assertInLoopThread();
    assert(state_ == StateE::kConnecting);
    setState(StateE::kConnected);
    channel_->tie(shared_from_this());
    channel_->enableReading();
    connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed() {
    loop_->assertInLoopThread();
    if (state_ == StateE::kConnected) {
        setState(StateE::kDisconnected);
        channel_->disableAll();
        connectionCallback_(shared_from_this());
    }
    channel_->removeChannel();
}

/*!
 * \private 私有成员函数
 * **********************************************************************************/
void TcpConnection::handleRead(Timestamp receiveTime) {
    loop_->assertInLoopThread();
    int saveError = 0;
    /** 从 socket缓冲区读取数据到 inputBuffer */
    ssize_t n = inputBuffer_.readFd(channel_->getFd(), &saveError);
    if (n > 0) {
        messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
    }
    /** 没读到数据 */
    else if (n == 0) {
        handleClose();
    }
    else {
        errno = saveError;
        handleError();
    }
}

void TcpConnection::handleWrite() {
    loop_->assertInLoopThread();
    if (channel_->isWriting()) {
        ssize_t n = tcpSocket::write(channel_->getFd(), outputBuffer_.peek(),
                                   outputBuffer_.readableBytes());
        if (n > 0) {
            outputBuffer_.retrieve(n);
            if (outputBuffer_.readableBytes() == 0) {
                channel_->disableWriting();
                if (writeCompleteCallback_) {
                    loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
                }
                if (state_ == StateE::kDisconnecting) {
                    shutdownInLoop();
                }
            }
        }
        /** n <= 0 */
        else {
            SPDLOG_ERROR("failed to write, please check");
        }
    }
    else {
        /** socket 连接关闭，没有数据写 */
    }
}

void TcpConnection::handleClose() {
    loop_->assertInLoopThread();
    assert(state_ == StateE::kConnected || state_ == StateE::kDisconnecting);
    setState(StateE::kDisconnected);
    channel_->disableAll();
    TcpConnectionPtr guardThis(shared_from_this());
    connectionCallback_(guardThis);  /* TODO */
    /** 必须在最后一行 */
    closeCallback_(guardThis);
}

void TcpConnection::handleError() {
    int err = tcpSocket::getSocketError(channel_->getFd());
    /** log error: err */
}

void TcpConnection::sendInLoop(const void *message, size_t len) {
    loop_->assertInLoopThread();
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool faultError = false;
    if (state_ == StateE::kDisconnected) {
        return;
    }
    /** 若buffer中没有数据，尝试直接发 */
    if ( !channel_->isWriting() && (outputBuffer_.readableBytes() == 0) ) {
        nwrote = tcpSocket::write(channel_->getFd(), message, len);
        /** 返回共发出去多少字节，或者返回-1（出错）*/
        if (nwrote >= 0) {
            remaining = len - nwrote;
            /** 全发出去了 */
            if (remaining == 0 && writeCompleteCallback_) {
                loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
            }
        }
        else {
            nwrote = 0;
            if (errno != EWOULDBLOCK) {
                /** log system error: TcpConnection::sendInLoop */
                if (errno == EPIPE || errno == ECONNRESET) // FIXME: any others?
                {
                    faultError = true;
                }
            }
        }
    }
    /** 追加到outputBuffer_
     * 1. outputBuffer_ 中本来就有数据，因此将新发送数据追加到buffer后面
     * 2. outputBuffer_ 中原本没有数据，本次发送时，一次没有完整发出去，将本次剩下的数据追加到buffer后面
     * */
    assert(remaining <= len);
    if (!faultError && remaining > 0) {
        ssize_t oldlen = outputBuffer_.readableBytes();
        /** outputBuffer 容量不够，执行高水位回调进行扩容 */
        if (oldlen + remaining >= highWaterMark_
            && oldlen < highWaterMark_
            && highWaterMarkCallback_) {
            loop_->queueInLoop(std::bind(highWaterMarkCallback_, shared_from_this(), oldlen + remaining));
        }
        /** 数据指针要加上 nwrote的原因：
         * 1. message全部追加到buffer中，此时 nwrote == 0
         * 2. message已经发出去了一部分，共计 nwrote， 那么数据起始指针需要向右移动 nwrote
         * */
        outputBuffer_.append(static_cast<const char*>(message) + nwrote, remaining);
        if (!channel_->isWriting()) {
            channel_->enableWriting();  /** 触发写事件 */
        }
    }
}

void TcpConnection::sendInLoop(const std::string &message) {
    sendInLoop(message.c_str(), message.size());
}

void TcpConnection::shutdownInLoop() {
    loop_->assertInLoopThread();
    if (!channel_->isWriting()) {
        socket_->shutdownWrite();
    }
}

void TcpConnection::forceCloseInLoop() {
    loop_->assertInLoopThread();
    if (state_ == StateE::kConnected || state_ == StateE::kDisconnecting) {
        handleClose();
    }
}

const char *TcpConnection::stateToString() const {
    switch (state_)
    {
        case StateE::kDisconnected:
            return "kDisconnected";
        case StateE::kConnecting:
            return "kConnecting";
        case StateE::kConnected:
            return "kConnected";
        case StateE::kDisconnecting:
            return "kDisconnecting";
        default:
            return "unknown state";
    }
}

void TcpConnection::startReadInLoop() {
    loop_->assertInLoopThread();
    if (!reading || !channel_->isReading()) {
        channel_->enableReading();
        reading = true;
    }
}

void TcpConnection::stopReadInLoop() {
    loop_->assertInLoopThread();
    if (reading || channel_->isReading()) {
        channel_->disableReading();
        reading = false;
    }
}