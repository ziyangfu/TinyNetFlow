//
// Created by fzy on 23-8-22.
//

#include "UdpServer.h"
#include "netflow/base/Logging.h"
#include "UdpSocketOps.h"
#include "SocketsOps.h"

using namespace netflow::net;
using namespace netflow::base;
/** static */ const int UdpServer::kBufferSize = 1400;

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
    if (addr.getFamiliy() == AF_INET6) {
        isV6_ = true;
    }
    else {
        isV6_ = false;
    }
    if (option_ == Option::kReusePort) {
        setReusePort(true);
    }
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

void UdpServer::setReusePort(bool on) {
    ::udpSockets::setUdpReusePort(sockfd_, on);
}

void UdpServer::setReuseAddr(bool on) {
    ::udpSockets::setUdpReuseAddr(sockfd_, on);
}

void UdpServer::start() {
    threadPool_->start(threadInitCallback_);
}

void UdpServer::close() {
    udpSockets::close(sockfd_);
}

void UdpServer::sendTo(const std::string &message, const InetAddr& clientAddr) {
    sendTo(message.c_str(), message.length(), clientAddr);
}

void UdpServer::sendTo(const char *data, size_t length, const InetAddr& clientAddr) {
    udpSockets::sendTo(sockfd_, clientAddr.getSockAddr(), data, length);
}

/** --------------------------------- 组播设置部分 ------------------------------------ */
void UdpServer::joinMulticastGroup(const netflow::net::InetAddr &multicastAddr) {
    if (isV6_) {
        const sockaddr_in6* addr6 = tcpSocket::sockaddr_in6_cast(multicastAddr.getSockAddr());
        udpSockets::joinMulticastGroupV6(sockfd_, addr6);
    }
    else {
        const sockaddr_in* addr =  tcpSocket::sockaddr_in_cast(multicastAddr.getSockAddr()) ;
        udpSockets::joinMulticastGroupV4(sockfd_, addr);
    }
}

void UdpServer::leaveMulticastGroup(const netflow::net::InetAddr &multicastAddr) {
    if (isV6_) {
        const sockaddr_in6* addr6 = tcpSocket::sockaddr_in6_cast(multicastAddr.getSockAddr());
        udpSockets::leaveMulticastGroupV6(sockfd_, addr6);
    }
    else {
        const sockaddr_in* addr =  tcpSocket::sockaddr_in_cast(multicastAddr.getSockAddr()) ;
        udpSockets::leaveMulticastGroupV4(sockfd_, addr);
    }
}

void UdpServer::setMulticastTTL(int ttl) {
    if (isV6_) {
        udpSockets::setMulticastTtlV6(sockfd_, ttl);
    }
    else {
        udpSockets::setMulticastTtlV4(sockfd_, ttl);
    }
}

void UdpServer::setMulticastInterface(const netflow::net::InetAddr &multicastAddr) {
    if (isV6_) {
        const sockaddr_in6* addr6 = tcpSocket::sockaddr_in6_cast(multicastAddr.getSockAddr());
        udpSockets::setMulticastNetworkInterfaceV6(sockfd_, addr6);
    }
    else {
        const sockaddr_in* addr =  tcpSocket::sockaddr_in_cast(multicastAddr.getSockAddr());
        udpSockets::setMulticastNetworkInterfaceV4(sockfd_, addr);
    }
}

void UdpServer::setMulticastLoop(bool on) {
    if (isV6_) {
        udpSockets::setMulticastLoopV6(sockfd_, on);
    }
    else {
        udpSockets::setMulticastTtlV4(sockfd_, on);
    }
}

/** --------------------------------- 回调函数部分 ------------------------------------ */
void UdpServer::handleRead(base::Timestamp receiveTime) {
    STREAM_TRACE << "exec in handleRead function";
    loop_->assertInLoopThread();
    char buffer[kBufferSize];
    sockaddr_in clientAddr{};
    ssize_t bytesRead = udpSockets::recvFrom(sockfd_, buffer,
                                     sizeof(buffer), (struct sockaddr*)&clientAddr);
    STREAM_TRACE << "receive data: " << buffer << ", size = " << bytesRead;
    InetAddr addr(clientAddr);
    // int saveError = 0;
    /** 从 socket缓冲区读取数据到 inputBuffer */
    // ssize_t n = receiveBuffer.readFd(sockfd_, &saveError);
    // std::string message = receiveBuffer.retrieveAllAsString();
    if (bytesRead > 0) {
        std::string message(buffer, bytesRead);
        messageCallback_(message, addr, receiveTime);
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

void UdpServer::handleClose() {

}

void UdpServer::handleError() {
    /** FIXME: temp */
    abort();
}
