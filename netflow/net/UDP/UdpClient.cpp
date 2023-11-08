//
// Created by fzy on 23-8-22.
//

#include "UdpClient.h"
#include "netflow/base/Logging.h"
#include "netflow/net/UDP/UdpSocketOps.h"



using namespace netflow::net;
using namespace netflow::base;

/** static */ const int UdpClient::kBufferSize = 1472;  /** 可能 1400 更好？ */

/** 1. 构造函数，创建UDP socket，并设置好参数
 *  2. 设置多播参数（加入多播组、设置多播TTL等）
 *  3. sendTo / recvFrom
 *  4. 析构函数， 删除fd */

UdpClient::UdpClient(EventLoop* loop, const InetAddr& serverAddr, const std::string& name)
    : sockfd_(udpSockets::createUdpSocket(serverAddr.getFamiliy())),
      loop_(loop),
      remoteAddr_(serverAddr),
      name_(name),
      isConnected_(false),
      buffer_(kBufferSize),
      channel_(nullptr)

{
    /** 1. 创建非阻塞UDP socket
     *  2. bind地址
     *  3. 设置消息回调函数
     *  4， new Channel，并注册epoll读事件 */
     udpSockets::setUdpReuseAddr(sockfd_, true);
     udpSockets::setUdpReusePort(sockfd_, true);

     channel_ = std::make_unique<Channel>(loop_, sockfd_);
     channel_->setReadCallback(std::bind(&UdpClient::handleRead, this, std::placeholders::_1));
     channel_->enableReading();
}

UdpClient::~UdpClient() {
    /** 1. 若加入多播组，则退出多播组
     *  2。 关闭socket fd */
    ::close(sockfd_);
}

bool UdpClient::connect() {
    int ret = udpSockets::connect(sockfd_, remoteAddr_.getSockAddr());
    if (ret != 0) {
        STREAM_ERROR << "failed to connect to remote addr via UDP protocol";
        close();
        return false;
    }
    isConnected_ = true;  /** UDP没有连接，这仅表示地址已经保存在内核中 */
    return true;
}

void UdpClient::close() {
    udpSockets::close(sockfd_);
}

bool UdpClient::send(const char *data, size_t length) {
    /**  若是 “ 已连接 ” 状态， 即提前保存了目标地址 */
    if (isConnected_) {
        ssize_t sendNum = ::send(sockfd_, data, length, 0);
        return static_cast<size_t>(sendNum) == length;
    }
    /** 没有提前设置地址，则设置地址，直接发送 */
    struct sockaddr* addr = reinterpret_cast<struct sockaddr*>(&remoteAddr_);
    socklen_t addrLength = sizeof(*addr);
    ssize_t sendNum = ::sendto(sockfd_, data, length, 0, addr, addrLength);
    return (sendNum > 0);
}

bool UdpClient::send(const std::string &message) {
    return send(message.data(), message.size());
}

std::string UdpClient::connectAndSend(const std::string &remoteIp, int port, const std::string &udpPackageData,
                               uint32_t timeoutMs) {
}


std::string UdpClient::sendAndReceive(const std::string &udpPackageData, uint32_t timeoutMs) {
    if (!send(udpPackageData)) {
        STREAM_ERROR << "send UDP message failed!";
        return "";
    }
    size_t bufferSize = 1472;
    socklen_t addrLength = sizeof(struct sockaddr);
    struct sockaddr* addr;
    int readNum = ::recvfrom(sockfd_, buffer_.beginWrite(), bufferSize,
                             0, addr, &addrLength);
    buffer_.retrieve(readNum);
    return buffer_.retrieveAllAsString();
}

void UdpClient::setMessageCallback(netflow::net::MessageCallback cb) {
    messageCallback_ = std::move(cb);
}

void UdpClient::handleRead(base::Timestamp receiveTime) {

}


void UdpClient::joinMulticastGroup() {
    if (remoteAddr_.getFamiliy() == AF_INET) {
        udpSockets::joinMulticastGroupV4(sockfd_, )
    }
    else if (remoteAddr_.getFamiliy() == AF_INET6){

    }
}

void UdpClient::leaveMulticastGroup() {

}

void UdpClient::setMulticastTTL(int ttl) {

}

void UdpClient::setMulticastInterface() {

}

void UdpClient::setMulticastLoop() {

}

