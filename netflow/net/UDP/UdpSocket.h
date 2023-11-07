//
// Created by fzy on 23-11-7.
//

#ifndef TINYNETFLOW_UDPSOCKET_H
#define TINYNETFLOW_UDPSOCKET_H

#include <arpa/inet.h>
#include "netflow/net/InetAddr.h"

namespace netflow::net {
class UdpSocket {
public:
    UdpSocket(int sockfd, bool isUseV6);
    ~UdpSocket();
    int getFd() const;

    void createAndBindSocket();
    void bind(const InetAddr& addr);
    int connect(int sockfd, const struct sockaddr* addr);

    void send();
    void sendTo();
    void sendAsync();

    void receive();
    void receiveAsync();

    void setMulticast();
private:
    const int sockfd_;

};
}



#endif //TINYNETFLOW_UDPSOCKET_H
