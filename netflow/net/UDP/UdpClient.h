//
// Created by fzy on 23-8-22.
//

#ifndef TINYNETFLOW_UDPCLIENT_H
#define TINYNETFLOW_UDPCLIENT_H

#include <memory>
#include <mutex>
#include<string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "netflow/net///UDP/UdpMessage.h"
#include "netflow/net//Buffer.h"

namespace netflow::net {

class UdpClient {
public:
    UdpClient(bool isUseV6 = false);
    ~UdpClient();

    bool connect(const char* host, int port);
    bool connect(const struct sockaddr& addr);
    bool connect(const struct sockaddr_in& addr);
    bool connect(const struct sockaddr_in6& addr6);
    void close();
    bool send(const std::string& message);
    bool send(const char* data, size_t length);

    std::string sendAndReceive(const std::string& udpPackageData, uint32_t timeoutMs);

    std::string connectAndSend(const std::string& remoteIp, int port,
                                 const std::string& udpPackageData, uint32_t timeoutMs);

    int getSockFd() const { return sockfd_; }
private:
    bool connect();

private:
    int sockfd_;
    bool  isConnected_;
    struct sockaddr_storage remoteAddr_;
    Buffer buffer_;
    static const int kBufferSize = 1472;
};


}


#endif //TINYNETFLOW_UDPCLIENT_H
