//
// Created by fzy on 23-8-22.
//

#ifndef TINYNETFLOW_UDPCLIENT_H
#define TINYNETFLOW_UDPCLIENT_H

#include <memory>
#include <functional>
#include <mutex>
#include<string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "netflow/net/InetAddr.h"
#include "netflow/net//Buffer.h"
#include "netflow/net/Channel.h"
#include "netflow/net/EventLoop.h"
#include "netflow/base/Timestamp.h"

namespace netflow::net {

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
    bool send(const std::string& message);
    bool send(const char* data, size_t length);

    std::string sendAndReceive(const std::string& udpPackageData, uint32_t timeoutMs);

    std::string connectAndSend(const std::string& remoteIp, int port,
                                 const std::string& udpPackageData, uint32_t timeoutMs);

    void setMessageCallback(messageCb cb);

    void joinMulticastGroup();
    void leaveMulticastGroup();
    void setMulticastTTL(int ttl);
    void setMulticastLoop();
    void setMulticastInterface();
private:
    void handleRead(base::Timestamp receiveTime);
    void handleClose();
    void handleError();

private:
    int sockfd_;
    EventLoop* loop_;
    InetAddr remoteAddr_;
    const std::string name_;
    bool  isConnected_;   /** 标识是否使用connect添加了远端地址， 若true则可以使用send */
    Buffer buffer_;
    messageCb messageCallback_;/** 消息回调 */;
    std::unique_ptr<Channel> channel_;

    static const int kBufferSize;
};

}


#endif //TINYNETFLOW_UDPCLIENT_H
