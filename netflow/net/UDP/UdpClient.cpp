//
// Created by fzy on 23-8-22.
//

#include "UdpClient.h"
#include "src/base/Logging.h"

using namespace netflow::net;
using namespace netflow::base;

UdpClient::UdpClient()
    : sockfd_(-1),
      isConnected_(false),
      buffer_(kBufferSize)
{
    memset(&remoteAddr_, 0, sizeof(remoteAddr_));
}

UdpClient::~UdpClient() {

}

bool UdpClient::connect(const char *host, int port) {

}

bool UdpClient::connect() {
    sockfd_ = ::socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr* addr = reinterpret_cast<struct sockaddr*>(&remoteAddr_);
    socklen_t addrLength = sizeof(*addr);
    int ret = ::connect(sockfd_, addr, addrLength);

    if (ret != 0) {
        STREAM_ERROR << "failed to connect to remote addr via UDP protocol";
        close();
        return false;
    }
    isConnected_ = true;  /** UDP没有连接，这仅表示地址已经保存在内核中 */
    return true;
}

void UdpClient::close() {
    ::close(sockfd_);
}

bool UdpClient::send(const char *data, size_t length) {
    /**  若是 “已连接” 状态， 即提前保存了目标地址 */
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
    if (!connect(remoteIp.data(), port)) {
        return "";
    }
    return sendAndReceive(udpPackageData, timeoutMs);
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

