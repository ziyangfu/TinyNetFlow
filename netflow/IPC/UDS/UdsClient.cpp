//
// Created by fzy on 23-11-7.
//

#include "UdsClient.h"
#include "netflow/base/Logging.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <cerrno>
#include <cstring>

using namespace netflow::net;

UdsClient::UdsClient(struct uds::UnixDomainPath path /** default is uds::UnixDomainDefaultPath */)
        : sockfd_(udsSockets::createUdsSocket()),
          path_(path),
          unixDomainPath_(generateUnixDomainPath())
{
}

UdsClient::~UdsClient() {
    ::close(sockfd_);
}

const std::string UdsClient::generateUnixDomainPath() {
    std::string str;
    if (path_.domain == 10 && path_.port == 10) {
        str = uds::kUnixDomainDefaultPathString;
    }
    else {
        str = uds::kUnixDomainPathFirstString + std::to_string(path_.domain)
                + uds::kUnixDomainPathSecondString + std::to_string(path_.port);
    }
    return str;
}

int UdsClient::getFd() const {
    return sockfd_;
}

int UdsClient::getDomain() const {
    return path_.domain;
}

int UdsClient::getPort() const {
    return path_.port;
}

const std::string &UdsClient::getUnixDomainAddr() const {
    return unixDomainPath_;
}

void UdsClient::bind() {

}