//
// Created by fzy on 23-11-7.
//

#include "UdsSocket.h"
#include "netflow/base/Logging.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <cerrno>
#include <cstring>

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
    ::close(sockfd_);
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
    if (fd < 0) {
        int serrno = errno;
        STREAM_ERROR << "failed to create Unix domain socket , the error = " << serrno;
        return -1;
    }
    return fd;
}

void UdsSocket::bind() {

    ::unlink(localAddr.c_str());  /** 预留。。。 */

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    std::strncpy(addr.sun_path, localAddr.c_str(), sizeof(addr.sun_path) - 1);
    if (::bind(sockfd_,(struct sockaddr*)&addr, sizeof(addr)) < 0) {
        STREAM_ERROR << "failed to bind Unix domain socket";
        ::close(sockfd_);
    }
}