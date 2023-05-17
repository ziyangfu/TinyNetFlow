//
// Created by fzy on 23-5-11.
//

#include "Socket.h"

#include <fcntl.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>

using namespace netflow;
using namespace netflow::net;

/** TODO:UDP支持 */
int Socket::createNonblockingSocket(sa_family_t family) {
    int sockfd = ::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if(sockfd < 0) {
        /** error */
    }
    return sockfd;
}

void Socket::bind(int sockfd, const struct sockaddr* addr) {
    int ret = ::bind(sockfd, addr, sizeof(struct sockaddr));
    if(ret < 0) {
        /** error */
    }
}

void Socket::listen(int sockfd) {
    int ret = ::listen(sockfd, SOMAXCONN);  /** listen队列最大长度 SOMAXCONN： 4096 */
    if(ret < 0) {
        /** error */
    }
}

int Socket::accept(int sockfd, const struct sockaddr* addr) {
    socklen_t addrlen = static_cast<socklen_t>(sizeof *addr);
    int connfd = ::accept4(sockfd, addr, &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if(connfd < 0) {
        /** error */
    }
    return connfd;
}

int Socket::connect(int sockfd, const struct sockaddr* addr) {
    return ::connect(sockfd, addr, static_cast<socklen_t>(sizeof (struct sockaddr_in6)));

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