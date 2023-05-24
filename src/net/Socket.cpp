//
// Created by fzy on 23-5-11.
//

#include "Socket.h"

#include "SocketsOps.h"

#include <fcntl.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>

using namespace netflow;
using namespace netflow::net;

/** TODO:UDP支持 */
int Socket::createNonblockingSocket(sa_family_t family) {
    return sockets::createNonblockingSocket(family);
}

void Socket::bind(int sockfd, const struct sockaddr* addr) {
    sockets::bind(sockfd, addr);
}

void Socket::listen(int sockfd) {
    sockets::listen(sockfd);
}

int Socket::accept(int sockfd, const struct sockaddr* addr) {
    return sockets::accept(sockfd, addr);
}

int Socket::connect(int sockfd, const struct sockaddr* addr) {
    return sockets::connect(sockfd, addr);
}
void Socket::shutdownWrite() {
    sockets::shutdownWrite(sockfd_);
}

/** set sock options */
void Socket::setTcpNoDelay(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY,
                 &optval, static_cast<socklen_t>(sizeof optval));
}

void Socket::setReuseAddr(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR,
                 &optval, static_cast<socklen_t>(sizeof optval));
}

void Socket::setReusePort(bool on) {
#ifdef SO_REUSEPORT
    int optval = on ? 1 : 0;
  int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT,
                         &optval, static_cast<socklen_t>(sizeof optval));
  if (ret < 0 && on)
  {
    //LOG_SYSERR << "SO_REUSEPORT failed.";
  }
#else
    if (on)
    {
        //LOG_ERROR << "SO_REUSEPORT is not supported.";
    }
#endif
}

void Socket::setKeepAlive(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE,
                 &optval, static_cast<socklen_t>(sizeof optval));
}

