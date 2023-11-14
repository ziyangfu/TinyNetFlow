//
// Created by fzy on 23-8-22.
//

#include "UdpClient.h"
#include "netflow/base/Logging.h"
#include "netflow/net/UDP/UdpSocketOps.h"

#include <functional>




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
    loop_->runInLoop([this](){
        //udpSockets::setUdpReuseAddr(sockfd_, true);
        //udpSockets::setUdpReusePort(sockfd_, true);
        channel_ = std::make_unique<Channel>(loop_, sockfd_);
        channel_->setReadCallback(std::bind(&UdpClient::handleRead, this, std::placeholders::_1));
        channel_->enableReading();
    });
}

UdpClient::~UdpClient() {
    /** 1. 若加入多播组，则退出多播组
     *  2. 关闭socket fd */
    close();
}
/*!
 * \brief 绑定客户端本地地址
 * */
void UdpClient::bind() {

}
/*!
 * \brief 保存服务端地址，connect后，可以直接使用 send/read/write，若不设置，则只能使用 sendTo/recvFrom
 * */
bool UdpClient::connect() {
    STREAM_TRACE << "connect function";
    loop_->runInLoop([this](){
        int ret = udpSockets::connect(sockfd_, remoteAddr_.getSockAddr());
        if (ret != 0) {
            STREAM_ERROR << "failed to connect to remote addr via UDP protocol";
            close();
            return false;
        }
        isConnected_ = true;  /** UDP没有连接，这仅表示地址已经保存在内核中 */
        return true;
    });
}

void UdpClient::close() {
    loop_->runInLoop([this](){
        udpSockets::close(sockfd_);
    });
}

void UdpClient::send(const char *data, size_t length) {
    loop_->runInLoop([this, data, length](){
        /**  若是 “ 已连接 ” 状态， 即提前保存了目标地址 */
        if (isConnected_) {
            STREAM_TRACE << "call udpSockets::send";
            //udpSockets::send(sockfd_, data, length);
            udpSockets::write(sockfd_, data, length);
        }
        else {
            /** 没有提前设置地址，则设置地址，直接发送 */
           udpSockets::sendTo(sockfd_, remoteAddr_.getSockAddr(), data, length);
        }
    });
}

void UdpClient::send(const std::string &message) {
    send(message.data(), message.size());
}
/** FIXME: 暂时没有用到 */
void UdpClient::sendInLoop(const void *message, size_t len) {
    loop_->assertInLoopThread();
}

std::string UdpClient::connectAndSend(const std::string &remoteIp, int port, const std::string &udpPackageData,
                               uint32_t timeoutMs) {
}


std::string UdpClient::sendAndReceive(const std::string &udpPackageData, uint32_t timeoutMs) {
#if 0
    if (!send(udpPackageData)) {
        STREAM_ERROR << "send UDP message failed!";
        return "";
    }
    size_t bufferSize = 1472;
    socklen_t addrLength = sizeof(struct sockaddr);
    struct sockaddr* addr;
    /** FIXME 此时socket是非阻塞IO，无法在这里阻塞，因此代码存在问题 */
    int readNum = ::recvfrom(sockfd_, buffer_.beginWrite(), bufferSize,
                             0, addr, &addrLength);
    buffer_.retrieve(readNum);
    return buffer_.retrieveAllAsString();
#endif
}

void UdpClient::setMessageCallback(messageCb cb) {
    messageCallback_ = std::move(cb);
}

void UdpClient::handleRead(base::Timestamp receiveTime) {
    STREAM_TRACE << "handleRead function";
    loop_->assertInLoopThread();
    sockaddr_in remoteAddr;
    char buffer[kBufferSize];
    int n = udpSockets::recvFrom(sockfd_, buffer ,
                                 sizeof(buffer), (struct sockaddr*)&remoteAddr);
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

void UdpClient::handleClose() {
    STREAM_TRACE << "handleClose function";
    loop_->assertInLoopThread();
    channel_->disableAll();
    close();
}


void UdpClient::handleError() {
    loop_->assertInLoopThread();
    /** ...... */
}

void UdpClient::joinMulticastGroup() {
    if (remoteAddr_.getFamiliy() == AF_INET) {
       // udpSockets::joinMulticastGroupV4(sockfd_, )
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

