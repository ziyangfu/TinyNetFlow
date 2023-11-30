//
// Created by fzy on 23-11-16.
//

#include "UdsServer.h"

#include "netflow/net/EventLoop.h"
#include "netflow/net/Channel.h"
#include "netflow/net/EventLoopThreadPool.h"

#include "netflow/base/Logging.h"

using namespace netflow::net;
using namespace netflow::base;

/** static */ const int UdsServer::kBufferSize = 1400;

UdsServer::UdsServer(EventLoop* loop, const std::string& name,
                     struct uds::UnixDomainPath path /** == uds::UnixDomainDefaultPath */)
    : sockfd_(udsSockets::createUdsSocket()),
      path_(path),
      unixDomainStringPath_(generateUnixDomainPath()),
      loop_(loop),
      name_(name),
      isConnected_(false),
      threadPool_(std::make_shared<EventLoopThreadPool>(loop_, name_))
{
    loop_->runInLoop([this](){
        //udpSockets::setUdpReuseAddr(sockfd_, true);
        //udpSockets::setUdpReusePort(sockfd_, true);
        bind();
        listen();
        //accept();
        channel_ = std::make_unique<Channel>(loop_, sockfd_);
        channel_->setReadCallback(std::bind(&UdsServer::onAccept, this));
        channel_->enableReading();
    });
}

UdsServer::~UdsServer() {
    close();
}

void UdsServer::start() {

}

void UdsServer::stop() {

}

void UdsServer::close() {
    loop_->runInLoop([this]() {
        channel_->disableAll();
        udsSockets::close(sockfd_);
    });
}

void UdsServer::bind() {
    udsSockets::bind(sockfd_, unixDomainStringPath_);
}

void UdsServer::listen() {
    udsSockets::listen(sockfd_);
}

int UdsServer::accept() {
    return udsSockets::accept(sockfd_, unixDomainStringPath_);
}

void UdsServer::send(const std::string &message) {
    if (loop_->isInLoopThread()) {
        sendInLoop(message);
    }
    else {
        void (UdsServer::*fp)(const std::string& message) = &UdsServer::sendInLoop;
        loop_->runInLoop(std::bind(fp, this, message));
    }
}

void UdsServer::setThreadNums(int threadNum) {
    assert(threadNum >= 0);
    threadPool_->setThreadNum(threadNum);
}

void UdsServer::setMessageCallback(netflow::net::UdsServer::messageCb cb) {
    messageCallback_ = std::move(cb);
}

std::string UdsServer::generateUnixDomainPath() {
    std::string str;
    if (path_.domain == 10 && path_.port == 10) {
        str = uds::kUnixDomainDefaultPathString;
    }
    else {
        str = uds::kUnixDomainPathFirstString + std::to_string(path_.domain)
              + uds::kUnixDomainPathSecondString + std::to_string(path_.port);
    }
    STREAM_TRACE << "unix domain socket path is " << str;
    return str;
}

/*!
 * \brief accept回调函数
 * */
void UdsServer::onAccept() {
    loop_->runInLoop([this](){
        /** 去除原来的channel */
        clientFd_ = accept();
        channel_->disableAll();
        channel_->removeChannel();
        //channel_.reset();
        /** 新channel */
        channel_.reset(new Channel(loop_, clientFd_));
        channel_->setReadCallback(std::bind(&UdsServer::handleRead, this, std::placeholders::_1));
        channel_->enableReading();
    });
}

/*!
 * \brief 在loop所在线程发送数据
 * \private
 * */
void UdsServer::sendInLoop(const std::string &message) {
    loop_->assertInLoopThread();
    sendInLoop(message.c_str(), message.length());
}

void UdsServer::sendInLoop(const void *message, size_t len) {
    /**  若是 “ 已连接 ” 状态， 即提前保存了目标地址 */
    if (isConnected_) {
        udsSockets::write(sockfd_, message, len);
    }
    else {
        /** 没有提前设置地址，报错 */
        STREAM_ERROR << "UDS ： must to call connect() to set remote address before now";
    }
}

void UdsServer::handleRead(base::Timestamp receiveTime) {
    loop_->assertInLoopThread();
    char buffer[kBufferSize];

    int bytesRead = udsSockets::read(sockfd_, buffer, sizeof(buffer));
    if (bytesRead > 0) {
        std::string message(buffer, bytesRead);
        messageCallback_(message, receiveTime);
    }
        /** 没读到数据 */
    else if (bytesRead == 0) {
        handleClose();
    }
    else {
        //errno = saveError;
        handleError();
    }
}

void UdsServer::handleClose() {

}

void UdsServer::handleError() {
    abort();
}