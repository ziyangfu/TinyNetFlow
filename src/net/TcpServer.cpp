//
// Created by fzy on 23-5-24.
//

#include "TcpServer.h"

#include "EventLoop.h"
#include "EventLoopThreadPool.h"
#include "Acceptor.h"
#include "SocketsOps.h"

using namespace netflow::net;

TcpServer::TcpServer(netflow::net::EventLoop *loop, const netflow::net::InetAddr &listenAddr,
                     const std::string &name, netflow::net::TcpServer::Option option)
    : loop_(loop),
      ipPort_(listenAddr.sockaddrToStringIpPort()),
      name_(name),
      acceptor_(std::make_unique<Acceptor>(loop_, listenAddr, option == kReusePort)),
      threadPool_(std::make_shared<EventLoopThreadPool>(loop_, name_)),
      connectionCallback_(defaultConnectionCallback),
      messageCallback_(defaultMessageCallback),
      nextConnId_(1),
      started_(false)
{
    acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this,
                                                  std::placeholders::_1, std::placeholders::_2));
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
void TcpServer::newConnection(int sockfd, const netflow::net::InetAddr &peerAddr) {
    loop_->assertInLoopThread();
    EventLoop* ioLoop = threadPool_->getNextLoop();
    char buf[64];
    snprintf(buf, sizeof buf, "-%s#%d", ipPort_.c_str(), nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;
    InetAddr localAddr{sockets::getLocalAddr(sockfd)};
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

void TcpServer::removeConnection(const netflow::net::TcpConnectionPtr &conn) {
    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const netflow::net::TcpConnectionPtr &conn) {
    loop_->assertInLoopThread();
    size_t  n = connections_.erase(conn->getName());
    //(void) n; //FIXME: ?????
    assert(static_cast<int>(n) == 1);  /** 清除了1个 */
    EventLoop* ioLoop = conn->getLoop();
    ioLoop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}