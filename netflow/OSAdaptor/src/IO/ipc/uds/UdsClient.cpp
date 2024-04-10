//
// Created by fzy on 23-11-7.
//

#include "IO/ipc/uds/UdsClient.h"

#include "IO/ipc/IpcMediaAddr.h"
#include <spdlog/spdlog.h>


using namespace netflow::osadaptor::ipc::uds;
using namespace netflow::osadaptor::net;

/** static */ const int UdsClient::kBufferSize = 1400;  /** unix 域套接字的缓存大小 */


UdsClient::UdsClient(EventLoop *loop, const std::string &name,
                     struct UnixDomainPath path /** default is uds::UnixDomainDefaultPath */)
        : sockfd_(udsSocket::createUdsSocket()),
          path_(path),
          unixDomainStringPath_(generateUnixDomainPath()),
          loop_(loop),
          name_(name),
          isConnected_(false)
{
    loop_->runInLoop([this](){
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
        if (udsSocket::connect(sockfd_, unixDomainStringPath_) == -1) {
            SPDLOG_ERROR("failed to connect unix domain socket path");
            close();
        }
        isConnected_ = true;  /** 没有连接，这仅表示地址已经保存在内核中 */
    });
}

void UdsClient::close() {
    loop_->runInLoop([this](){
        channel_->disableAll();
        udsSocket::close(sockfd_);
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
    SPDLOG_ERROR("Not completed yet");
}

void UdsClient::setMessageCallback(UdsClient::messageCb cb) {
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
    SPDLOG_TRACE("unix domain socket path is {}", str);
    return str;
}

void UdsClient::sendInLoop(const std::string &message) {
    loop_->assertInLoopThread();
    sendInLoop(message.c_str(), message.length());
}

void UdsClient::sendInLoop(const void *message, size_t len) {
    /**  若是 “ 已连接 ” 状态， 即提前保存了目标地址 */
    if (isConnected_) {
        udsSocket::write(sockfd_, message, len);
    }
    else {
        /** 没有提前设置地址，报错 */
        SPDLOG_ERROR("uds ： must to call connect() to set remote address before now");
    }
}

void UdsClient::handleRead(time::Timestamp receiveTime) {
    loop_->assertInLoopThread();
    char buffer[kBufferSize];
    int n = udsSocket::read(sockfd_, buffer, sizeof(buffer));
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
    SPDLOG_TRACE("unix domain socket handle error event , close now");
    loop_->assertInLoopThread();
    close();
}

void UdsClient::handleClose() {
    SPDLOG_TRACE("unix domain socket handle close event , close now");
    loop_->assertInLoopThread();
    close();
}