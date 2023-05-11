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

int Socket::createNonblockingSocket(sa_family_t family) {
    int sockfd = ::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if(sockfd < 0) {
        /** error */
    }
    return sockfd;
}

void Socket::bind(int sockfd, const struct sockaddr* addr) {
    int ret = ::bind(sockfd, addr, static_cast<socklen_t>(sizeof sockaddr_in6));

}

void Socket::listen() {

}

int Socket::accept() {
    return 0;
}

void Socket::connect() {

}

/** set sock options */
void Socket::setTcpNoDelay(bool on) {

}

void Socket::setReuseAddr(bool on) {

}

void Socket::setReusePort(bool on) {

}

void Socket::setKeepAlive(bool on) {

}