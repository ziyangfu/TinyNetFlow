//
// Created by fzy on 23-11-16.
//

#include "UdsSocketOps.h"
#include "netflow/net/SocketsOps.h"
#include "netflow/base/Logging.h"

#include <cstring>

using namespace netflow::net;
using namespace netflow::base;

int udsSockets::createUdsSocket() {
    int sockfd = ::socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (sockfd < 0) {
        STREAM_ERROR << "failed to create unix domain socket socket ";
    }
    return sockfd;
}

void udsSockets::bind(int fd, const std::string& path) {
    sockaddr_un unixAddr{};
    unixAddr.sun_family = AF_UNIX;
    std::strncpy(unixAddr.sun_path, path.c_str(), sizeof(unixAddr.sun_path) - 1);
    unlink(path.c_str());  /** 删除之前创建的套接字文件 */
    if (bind(fd, (struct sockaddr*)&unixAddr, sizeof(unixAddr)) == -1) {
        STREAM_ERROR << "failed to bind unix domain socket path";
    }
}

void udsSockets::listen(int sockfd) {
    sockets::listen(sockfd);
}

int udsSockets::accept(int sockfd, struct sockaddr_in6* addr) {
    return sockets::accept(sockfd,addr);
}

int udsSockets::connect(int fd, const struct sockaddr *addr) {
    int ret = ::connect(fd, addr, sizeof(struct sockaddr));
    return ret;
}

ssize_t udsSockets::read(int fd, void *buf, size_t count) {
    return ::read(fd, buf, count);
}


ssize_t udsSockets::write(int fd, const void *buf, size_t count) {
    return ::write(fd, buf, count);
}

int udsSockets::close(int sockfd) {
    return ::close(sockfd);
}


