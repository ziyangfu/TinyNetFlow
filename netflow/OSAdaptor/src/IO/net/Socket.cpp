//
// Created by fzy on 23-5-11.
//

#include "Socket.h"
#include "SocketsOps.h"

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <cstring>


using namespace netflow::net;

Socket::~Socket() {
    tcpSocket::close(sockfd_);
}

void Socket::bindAddr(const InetAddr& localAddr) {
    tcpSocket::bind(sockfd_, localAddr.getSockAddr());
}

void Socket::listen() {
    tcpSocket::listen(sockfd_);
}

int Socket::accept(InetAddr* peerAddr) {
    struct sockaddr_in6 addr;
    memset(&addr, 0, sizeof addr);
    int connfd = tcpSocket::accept(sockfd_, &addr);
    if (connfd >= 0)
    {
        peerAddr->setSockAddrInet6(addr);
    }
    return connfd;
}


void Socket::shutdownWrite() {
    tcpSocket::shutdownWrite(sockfd_);
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

