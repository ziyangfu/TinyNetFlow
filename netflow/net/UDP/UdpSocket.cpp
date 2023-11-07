//
// Created by fzy on 23-11-7.
//

#include "UdpSocket.h"
#include "netflow/base/Logging.h"
#include <cerrno>

using namespace netflow::net;

UdpSocket::UdpSocket(int sockfd, bool isUseV6)
    : sockfd_(sockfd)
{

}

int UdpSocket::getFd() const {
    return sockfd_;
}

void UdpSocket::bind(const InetAddr& addr) {
    const sockaddr* localAddr = addr.getSockAddr();
    if (::bind(sockfd_, localAddr, sizeof(localAddr)) < 0) {
        int serrno = errno;
        STREAM_ERROR << "failed to bind UDP socket, the errno = " << strerror(serrno);
    }
}

int UdpSocket::connect(int sockfd, const struct sockaddr* addr) {
    int ret = ::connect(sockfd, addr, sizeof(struct sockaddr));
    return ret;
}