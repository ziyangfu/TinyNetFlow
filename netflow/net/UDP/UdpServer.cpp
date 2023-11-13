//
// Created by fzy on 23-8-22.
//

#include "UdpServer.h"
#include "netflow/base/Logging.h"
#include "netflow/net/UDP/UdpSocketOps.h"
#include "netflow/net/SocketsOps.h"

using namespace netflow::net;
using namespace netflow::base;
/** static */ const int UdpServer::kBufferSize = 1472;  /** 可能 1400 更好？ */

UdpServer::UdpServer(netflow::net::EventLoop *loop,
                     const netflow::net::InetAddr &addr,
                     const std::string &name,
                     netflow::net::UdpServer::Option option)
         : sockfd_(udpSockets::createUdpSocket(addr.getFamiliy())),
           loop_(loop),
           localAddr_(addr),
           name_(name),
           option_(option),
           channel_(nullptr),
           receiveBuffer(kBufferSize),
           status_(Status::kStopped),
           ipPort_(addr.toIpPort()),
           threadPool_(std::make_shared<EventLoopThreadPool>(loop_, name_))
{
    /** 1. 创建socket
     *  2. bind本地地址
     *  3. 绑定channel
     *  4. 触发读事件 */
     udpSockets::bind(sockfd_, localAddr_.getSockAddr());
     channel_ = std::make_unique<Channel>(loop_, sockfd_);
     channel_->setReadCallback(std::bind(&UdpServer::handleRead, this, std::placeholders::_1));
     channel_->enableReading();
}

UdpServer::~UdpServer() {

}

void UdpServer::setThreadNum(int numThreads) {
    assert(numThreads >= 0);
    threadPool_->setThreadNum(numThreads);
}

void UdpServer::start() {
    threadPool_->start(threadInitCallback_);
}

void UdpServer::close() {
    udpSockets::close(sockfd_);
}

bool UdpServer::sendTo(const std::string &message, const InetAddr& clientAddr) {
    return sendTo(message.c_str(), message.length(), clientAddr);
}

bool UdpServer::sendTo(const char *data, size_t length, const InetAddr& clientAddr) {
    return udpSockets::sendTo(sockfd_, clientAddr.getSockAddr(), data, length);
}

void UdpServer::handleRead(base::Timestamp receiveTime) {
    loop_->assertInLoopThread();
    sockaddr* clientAddr;
    std::string message;
    int n = udpSockets::recvFrom(sockfd_, (void*)message.c_str(),
                               message.length(), clientAddr);
    const sockaddr_in* p =  sockets::sockaddr_in_cast(clientAddr);
    InetAddr addr(*p);
    // int saveError = 0;
    /** 从 socket缓冲区读取数据到 inputBuffer */
    // ssize_t n = receiveBuffer.readFd(sockfd_, &saveError);
    // std::string message = receiveBuffer.retrieveAllAsString();
    if (n > 0) {
        messageCallback_(message, addr, receiveTime);
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

void UdpServer::handleClose() {

}

void UdpServer::handleError() {

}
