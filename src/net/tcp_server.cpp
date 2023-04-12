//
// Created by fzy on 23-3-13.
//

#include "tcp_server.h"

#include "../base/logging.h"
#include "acceptor.h"
#include "event_loop.h"
#include "event_loop_thread_pool.h"
#include "socket_ops.h"

#include <stdio.h> // snprintf

using namespace muduo;
using namespace muduo::net;

TcpServer::TcpServer(muduo::net::EventLoop *loop, const muduo::net::InetAddress &listenAddr,
                     const std::string &nameArg, muduo::net::TcpServer::Option option)
         : loop_(CHECK_NOTNULL(loop)),
           ipPort_(listenAddr.toIpPort()),
           name_(nameArg),
           acceptor_(new Acceptor(loop, listenAddr, option == kReusePort)),
           threadPool_(new EventLoopThreadPool(loop, name_)),
           connectionCallback_(defaultConnectionCallback),
           messageCallback_(defaultMessageCallback),
           nextConnId_(1)
{
    acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this, _1, _2));
    /*
    acceptor_->setNewConnectionCallback(
     [](int sockfd, const muduo::net::InetAddress &peerAddr){
        // newConnection 内容
    });
    */
}

TcpServer::~TcpServer() {
    loop_->assertInLoopThread();
    LOG_TRACE << "TcpServer::~TcpServer [" << name_ << "] destructing";

    for (auto& item : connections_)
    {
        TcpConnectionPtr conn(item.second);
        item.second.reset();
        conn->getLoop()->runInLoop(
                std::bind(&TcpConnection::connectDestroyed, conn));
    }
}

void TcpServer::setThreadNum(int numThreads)
{
    assert(0 <= numThreads);
    /**
     * 1. numThreads > 1 : 多 Reactor 模式
     * 2. numThreads = 0 : 单 Reactor 模式 */
    threadPool_->setThreadNum(numThreads);
}

void TcpServer::start()
{
    /** 原子操作可以使用 std::atomic_bool */
    if (started_.getAndSet(1) == 0)
    {
        threadPool_->start(threadInitCallback_);
        assert(!acceptor_->listening());
        loop_->runInLoop(
                std::bind(&Acceptor::listen, get_pointer(acceptor_)));
    }
}
/** 将 TCP 连接绑定到 event loop (sub Reactor)上 */
void TcpServer::newConnection(int sockfd, const muduo::net::InetAddress &peerAddr) {
    loop_->assertInLoopThread();
    /*! 时间片轮转调度 */
    EventLoop* ioLoop = threadPool_->getNextLoop();
    char buf[64];
    snprintf(buf, sizeof buf, "-%s#%d", ipPort_.c_str(), nextConnId_);
    ++nextConnId_;
    string connName = name_ + buf;

    LOG_INFO << "TcpServer::newConnection [" << name_
             << "] - new connection [" << connName
             << "] from " << peerAddr.toIpPort();
    InetAddress localAddr(sockets::getLocalAddr(sockfd));
    // FIXME poll with zero timeout to double confirm the new connection
    // FIXME use make_shared if necessary
    TcpConnectionPtr conn(new TcpConnection(ioLoop,
                                            connName,
                                            sockfd,
                                            localAddr,
                                            peerAddr));
    connections_[connName] = conn;
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(
            std::bind(&TcpServer::removeConnection, this, _1)); // FIXME: unsafe
    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn)
{
    // FIXME: unsafe
    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn)
{
    loop_->assertInLoopThread();
    LOG_INFO << "TcpServer::removeConnectionInLoop [" << name_
             << "] - connection " << conn->name();
    size_t n = connections_.erase(conn->name());
    (void)n;
    assert(n == 1);
    EventLoop* ioLoop = conn->getLoop();
    ioLoop->queueInLoop(
            std::bind(&TcpConnection::connectDestroyed, conn));
}