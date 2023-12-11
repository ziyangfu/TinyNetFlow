//
// Created by fzy on 23-5-24.
//

#include "TcpClient.h"

#include "netflow/OSLayer/IO/reactor/EventLoop.h"
#include "Connector.h"
#include "SocketsOps.h"

#include <stdio.h> // for snprintf

using namespace netflow::net;

namespace netflow::net::details {

void removeConnection(EventLoop* loop, const TcpConnectionPtr& conn)
{
    loop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}
}  // namespace netflow::net::details
TcpClient::TcpClient(netflow::net::EventLoop *loop, const netflow::net::InetAddr &serverAddr,
                     const std::string &name)
    : loop_(loop),
      name_(name),
      connector_(std::make_shared<Connector>(loop, serverAddr)),
      connectionCallback_(defaultConnectionCallback),
      messageCallback_(defaultMessageCallback),
      retry_(false),
      connect_(true),
      nextConnId_(1),
      mutex_()
 {
    connector_->setNewConnectionCallback(std::bind(&TcpClient::newConnection, this, std::placeholders::_1));
    /* FIXME: setConnectionFailedCallback */

 }

 TcpClient::~TcpClient() {
    TcpConnectionPtr conn;
    bool unique = false;

    std::unique_lock<std::mutex> lock(mutex_);
    /** 检查所管理对象是否仅由当前 shared_ptr 的实例管理, 返回值： bool */
    unique = connection_.unique();
    conn = connection_;
    lock.unlock();

    if (conn) {
        assert(loop_ == conn->getLoop());
        CloseCallback cb = std::bind(&details::removeConnection, loop_, std::placeholders::_1);

        loop_->runInLoop(std::bind(&TcpConnection::setCloseCallback, conn, cb));
        if (unique) {
            conn->forceClose();
        }
    }
    else {
        connector_->stop();
        /** TODO： 一段时间后停止
         * loop_->runAfter() */
    }
}

void TcpClient::connect() {
    connect_ = true;
    connector_->start();
}
/*!
 * \brief 关闭客户端连接写功能，读功能保持，连接保持 */
void TcpClient::disconnect() {
    connect_ = false;

    std::unique_lock<std::mutex> lock(mutex_);
    if (connection_) {
        connection_->shutdown();  /** 关闭写功能 */
    }
}
/*!
 * \brief 完全关闭，将socket描述符移出epoll监控范围 */
void TcpClient::stop() {
    connect_ = false;
    connector_->stop();
}

/*!
 * \brief 当连接建立后执行
 * \arg sockfd:已经建立连接后的sockfd
 * \private */
void TcpClient::newConnection(int sockfd) {
    loop_->assertInLoopThread();
    InetAddr peerAddr(sockets::getPeerAddr(sockfd));
    char buf[64];

    snprintf(buf, sizeof buf, "-%s#%d", peerAddr.toIpPort().c_str(), nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;
    InetAddr localAddr{sockets::getLocalAddr(sockfd)};
    TcpConnectionPtr conn{std::make_shared<TcpConnection>(loop_, connName, sockfd,
                                                          localAddr, peerAddr)};
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(std::bind(&TcpClient::removeConnection, this, std::placeholders::_1));
    /** TODO highWaterMarkCallback??? */

    std::unique_lock<std::mutex> lock(mutex_);
    connection_ = conn;
    lock.unlock();

    conn->connectEstablished();
}

void TcpClient::removeConnection(const netflow::net::TcpConnectionPtr &conn) {
    loop_->assertInLoopThread();
    assert(loop_ == conn->getLoop());

    std::unique_lock<std::mutex> lock(mutex_);
    assert(connection_ == conn);
    connection_.reset();
    lock.unlock();

    loop_->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
    if (retry_ && connect_) {
        connector_->restart();
    }
}

TcpConnectionPtr TcpClient::getTcpConnectionPtr()  {
    std::unique_lock<std::mutex> lock(mutex_);  /**  为什么要加锁 */
    return connection_;
}