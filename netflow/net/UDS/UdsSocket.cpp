//
// Created by fzy on 23-11-7.
//

#include "UdsSocket.h"
#include "netflow/base/Logging.h"
#include <sys/socket.h>
#include <cerrno>

using namespace netflow::net;

UdsSocket::UdsSocket(int domain, int port)
    : domain_(domain),
      port_(port),
      sockfd_(createSocket()),
      localAddr(uds::udsAddrFirst + std::to_string(domain_)
                + uds::udsAddrSecond + std::to_string(port_))
{

}

UdsSocket::~UdsSocket() {

}

int UdsSocket::getFd() const {
    return sockfd_;
}

int UdsSocket::getDomain() const {
    return domain_;
}

int UdsSocket::getPort() const {
    return port_;
}

const std::string &UdsSocket::getLocalAddr() const {
    return localAddr;
}

int UdsSocket::createSocket() {
    int fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd == -1) {
        int serrno = errno;
        STREAM_ERROR << "failed to create Unix domain socket , the error = " << serrno;
    }
    return fd;
}

void UdsSocket::bind() {
    if (::bind(sockfd_,))
}