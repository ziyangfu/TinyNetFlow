//
// Created by fzy on 23-5-17.
//

#include "Connector.h"

#include "Channel.h"
#include "EventLoop.h"
#include "SocketsOps.h"
#include "../base/Logging.h"

#include <errno.h>
#include <assert.h>
#include <string.h>

using namespace netflow::net;
using namespace netflow::base;

const int Connector::kMaxRetryDelayMs;
Connector::Connector(netflow::net::EventLoop *loop, const netflow::net::InetAddr &serverAddr)
    : loop_(loop),
      serverAddr_(serverAddr),
      connect_(false),
      state_(kDisconnected),
      retryDelayMs_(kInitRetryDelayMs)
{}
Connector::~Connector() {

}

void Connector::start() {
    connect_ = true;
    /** 此时调用runInLoop的线程不是loop线程，因此需要进行唤醒， startInLoop实际在doPendingFunctors中执行 */
    loop_->runInLoop(std::bind(&Connector::startInLoop, this));
}

void Connector::startInLoop() {
    loop_->assertInLoopThread();
    assert(state_ == kDisconnected);
    if (connect_)
    {
        connect();
    }
    else
    {
        STREAM_DEBUG << "do not connect";
    }
}

void Connector::stop()
{
    connect_ = false;
    loop_->queueInLoop(std::bind(&Connector::stopInLoop, this)); // FIXME: unsafe
    // FIXME: cancel timer
}

void Connector::stopInLoop()
{
    loop_->assertInLoopThread();
    if (state_ == kConnecting)
    {
        setState(kDisconnected);
        int sockfd = removeAndResetChannel();
        retry(sockfd);
    }
}

void Connector::connect()
{
    int sockfd = sockets::createNonblockingSocket(serverAddr_.getFamiliy());  // 创建 socket
    int ret = sockets::connect(sockfd, serverAddr_.getSockAddr());   // 建立连接
    int savedErrno = (ret == 0) ? 0 : errno;
    switch (savedErrno)
    { // 当前连接已经建立成功、正在进行中，或者被信号中断
        case 0:  // 建立成功
        case EINPROGRESS:  // TCP 三次握手仍在继续
        case EINTR:
        case EISCONN:
            connecting(sockfd);  // 处理连接结果
            break;

        case EAGAIN:  // 在非阻塞模式下，连接不可立即建立，需要重试
        case EADDRINUSE:
        case EADDRNOTAVAIL:
        case ECONNREFUSED:
        case ENETUNREACH:
            retry(sockfd); // 重试连接
            break;

        case EACCES: // 其他错误码表示连接失败，关闭套接字并记录错误日志
        case EPERM:
        case EAFNOSUPPORT:
        case EALREADY:
        case EBADF:
        case EFAULT:
        case ENOTSOCK:
            STREAM_ERROR << "connect error in Connector::startInLoop " << savedErrno;
            sockets::close(sockfd);
            break;

        default:
            STREAM_ERROR << "Unexpected error in Connector::startInLoop " << savedErrno;
            sockets::close(sockfd);
            // connectErrorCallback_();
            break;
    }
}

void Connector::restart()
{
    loop_->assertInLoopThread();
    setState(kDisconnected);
    retryDelayMs_ = kInitRetryDelayMs;
    connect_ = true;
    startInLoop();
}

void Connector::connecting(int sockfd)   // socket连接建立后，处理上层
{
    setState(kConnecting);
    assert(!channel_);
    channel_.reset(new Channel(loop_, sockfd));
    channel_->setWriteCallback(  // 建立TCP连接阶段时的写回调
            std::bind(&Connector::handleWrite, this)); // FIXME: unsafe
    channel_->setErrorCallback(
            std::bind(&Connector::handleError, this)); // FIXME: unsafe

    // channel_->tie(shared_from_this()); is not working,
    // as channel_ is not managed by shared_ptr
    channel_->enableWriting();   // 最终会把channel添加到epoll中
}

int Connector::removeAndResetChannel()
{
    channel_->disableAll();  // 清除事件
    channel_->removeChannel();   // 从 epoll中移除 channel
    int sockfd = channel_->getFd();  // 保存fd
    // Can't reset channel_ here, because we are inside Channel::handleEvent
    loop_->queueInLoop(std::bind(&Connector::resetChannel, this)); // FIXME: unsafe
    return sockfd;
}

void Connector::resetChannel()
{
    channel_.reset();  // 释放当前指针所拥有的对象，并将channel_置为nullptr
}

void Connector::handleWrite()
{
    STREAM_TRACE << "Connector::handleWrite " << state_;

    if (state_ == kConnecting)
    {
        int sockfd = removeAndResetChannel();  // 为什么要移除重置？？ 因为连接阶段结束，这些事件不需要了，而上层还需要sockfd
        int err = sockets::getSocketError(sockfd); // 获取套接字上待处理的错误数量，实际作用是再次确认是否成功建立连接
        if (err)
        {
            STREAM_WARN << "Connector::handleWrite - SO_ERROR = "
                     << err;
            retry(sockfd);
        }
        else if (sockets::isSelfConnect(sockfd))
        {
            STREAM_WARN << "Connector::handleWrite - Self connect";
            retry(sockfd);
        }
        else
        {
            setState(kConnected);
            if (connect_)
            {
                newConnectionCallback_(sockfd);   // 执行 TcpClient中的 newConnection()
            }
            else
            {
                sockets::close(sockfd);
            }
        }
    }
    else
    {
        // what happened?
        assert(state_ == kDisconnected);
    }
}

void Connector::handleError()
{
    STREAM_ERROR << "Connector::handleError state=" << state_;
    if (state_ == kConnecting)
    {
        int sockfd = removeAndResetChannel();
        int err = sockets::getSocketError(sockfd);
        STREAM_TRACE << "SO_ERROR = " << err << " ";
        retry(sockfd);
    }
}

void Connector::retry(int sockfd)
{
    sockets::close(sockfd);
    setState(kDisconnected);
    if (connect_)
    {
        STREAM_INFO << "Connector::retry - Retry connecting to " << serverAddr_.toIpPort()
                 << " in " << retryDelayMs_ << " milliseconds. ";
        loop_->runAfter(retryDelayMs_/1000.0,
                        std::bind(&Connector::startInLoop, shared_from_this()));
        retryDelayMs_ = std::min(retryDelayMs_ * 2, kMaxRetryDelayMs);
    }
    else
    {
        STREAM_DEBUG << "do not connect";
    }
}