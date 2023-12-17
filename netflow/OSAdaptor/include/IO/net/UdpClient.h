//
// Created by fzy on 23-8-22.
//

#ifndef TINYNETFLOW_UDPCLIENT_H
#define TINYNETFLOW_UDPCLIENT_H

#include <memory>
#include <functional>
#include <mutex>
#include <string>
#include <atomic>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "InetAddr.h"
#include "Buffer.h"
#include "netflow/OSAdaptor/include/IO/reactor/Channel.h"
#include "netflow/OSAdaptor/include/IO/reactor/EventLoop.h"
#include "netflow/base/Timestamp.h"

namespace netflow::net {
/*!
 * \brief 非阻塞UDP套接字 + epoll */
class UdpClient {
public:
    using messageCb =  std::function<void (const std::string& message,
                                           netflow::base::Timestamp receiveTime)>;

    UdpClient(EventLoop* loop, const InetAddr& serverAddr, const std::string& name);
    ~UdpClient();
    int getSockFd() const { return sockfd_; }
    const std::string& getName() const { return name_; }

    void bind();
    bool connect();
    void close();
    void send(const std::string& message);
    void send(const char* data, size_t length);
    std::string sendAndReceive(const std::string& udpPackageData, uint32_t timeoutMs);
    std::string connectAndSend(const std::string& remoteIp, int port,
                                 const std::string& udpPackageData, uint32_t timeoutMs);
    void setMessageCallback(messageCb cb);



    void setMulticastLoop(bool on);

private:
    void handleRead(base::Timestamp receiveTime);
    void handleClose();
    void handleError();
    void sendInLoop(const void *message, size_t len);
    void sendInLoop(const std::string& message);

private:
    int sockfd_;
    EventLoop* loop_;
    InetAddr remoteAddr_;
    const std::string name_;
    std::atomic_bool isConnected_;   /** 标识是否使用connect添加了远端地址， 若true则可以使用send */
    Buffer buffer_;
    messageCb messageCallback_;/** 消息回调 */;
    std::unique_ptr<Channel> channel_;

    static const int kBufferSize;
};

}


#endif //TINYNETFLOW_UDPCLIENT_H
