//
// Created by fzy on 23-11-7.
//

#include "UdsClient.h"
#include "netflow/base/Logging.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <cerrno>
#include <cstring>

using namespace netflow::net;

/** static */ const int UdsClient::kBufferSize = 1400;  /** unix 域套接字的缓存大小 */

UdsClient::UdsClient(netflow::net::EventLoop *loop, const std::string &name,
                     struct uds::UnixDomainPath path /** default is uds::UnixDomainDefaultPath */)
        : sockfd_(udsSockets::createUdsSocket()),
          path_(path),
          unixDomainStringPath_(generateUnixDomainPath()),
          loop_(loop),
          name_(name),
          isConnected_(false)
{
    loop_->runInLoop([this](){
        //udpSockets::setUdpReuseAddr(sockfd_, true);
        //udpSockets::setUdpReusePort(sockfd_, true);
        channel_ = std::make_unique<Channel>(loop_, sockfd_);
        channel_->setReadCallback(std::bind(&UdsClient::handleRead, this, std::placeholders::_1));
        channel_->enableReading();
    });
}

UdsClient::~UdsClient() {
    close();
}

void UdsClient::connect() {
    loop_->runInLoop([this](){
        if (udsSockets::connect(sockfd_, unixDomainStringPath_) == -1) {
            STREAM_ERROR << "failed to connect unix domain socket path";
            close();
            return false;
        }
        isConnected_ = true;  /** 没有连接，这仅表示地址已经保存在内核中 */
        return true;
    });
}

void UdsClient::close() {
    loop_->runInLoop([this](){
        channel_->disableAll();
        udsSockets::close(sockfd_);
    });
}

void UdsClient::send(const std::string &message) {
    if (loop_->isInLoopThread()) {
        sendInLoop(message);
    }
    else {
        void (UdsClient::*fp)(const std::string& message) = &UdsClient::sendInLoop;
        loop_->runInLoop(std::bind(fp, this, message));
    }
}
/*!
 * \brief 暂未实现 */
void UdsClient::send(const char *data, size_t length) {
    STREAM_ERROR << "Not completed yet";
}

void UdsClient::setMessageCallback(netflow::net::UdsClient::messageCb cb) {
    messageCallback_ = std::move(cb);
}

int UdsClient::getFd() const {
    return sockfd_;
}

const std::string &UdsClient::getName() const {
    return name_;
}

int UdsClient::getDomain() const {
    return path_.domain;
}

int UdsClient::getPort() const {
    return path_.port;
}

const std::string &UdsClient::getUnixDomainAddr() const {
    return unixDomainStringPath_;
}

/** -------------------------------   private  ------------------------------------------------ */
std::string UdsClient::generateUnixDomainPath() {
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

void UdsClient::sendInLoop(const std::string &message) {
    loop_->assertInLoopThread();
    sendInLoop(message.c_str(), message.length());
}

void UdsClient::sendInLoop(const void *message, size_t len) {
    /**  若是 “ 已连接 ” 状态， 即提前保存了目标地址 */
    if (isConnected_) {
        udsSockets::write(sockfd_, message, len);
    }
    else {
        /** 没有提前设置地址，报错 */
        STREAM_ERROR << "UDS ： must to call connect() to set remote address before now";
    }
}

void UdsClient::handleRead(base::Timestamp receiveTime) {
    STREAM_TRACE << "handleRead function";
    loop_->assertInLoopThread();
    char buffer[kBufferSize];
    int n = udsSockets::read(sockfd_, buffer, sizeof(buffer));
    /** 从 socket缓冲区读取数据到 inputBuffer */
    //int saveError = 0;
    // ssize_t n = buffer_.readFd(sockfd_, &saveError);
    //std::string message = buffer_.retrieveAllAsString();
    if (n > 0) {
        std::string message(buffer, n);
        messageCallback_(message, receiveTime);
    }
        /** 没读到数据 */
    else if (n == 0) {
        handleClose();
    }
    else {
        //errno = saveError;
        handleError();
    }
}

void UdsClient::handleError() {
    STREAM_TRACE << "unix domain socket handle error event , close now";
    loop_->assertInLoopThread();
    close();
}

void UdsClient::handleClose() {
    STREAM_TRACE << "unix domain socket handle close event , close now";
    loop_->assertInLoopThread();
    close();
}