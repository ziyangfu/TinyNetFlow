//
// Created by fzy on 23-11-16.
//

#include "UdsSocketOps.h"
#include "netflow/net/SocketsOps.h"
#include "netflow/base/Logging.h"

#include <cstring>

using namespace netflow::net;
using namespace netflow::base;

/*!
 * \brief  创建一个非阻塞的流式unix域套接字
 * \details
 *      1. AF_UNIX 与 AF_LOCAL 同义
 *      2. 可以通过 SOCK_STREAM 创建流式套接字， 类似于TCP， 建立连接后使用read、write
 *      3. 可以通过 SOCK_DGRAM 创建报式套接字，类似于UDP, 后使用 sendto 与recvfrom
 *      4. SOCK_NONBLOCK设置为非阻塞模式，也可以默认设置为阻塞模式，然后写一个setBlocking函数，通过 fnctl 设置是否非阻塞模式
 *      5. SOCK_CLOEXEC是 close-on-exec机制，在fork子进程中执行exec的时候，会清理掉父进程创建的socket
 *      */
int udsSockets::createUdsSocket() {
    int sockfd = ::socket(AF_UNIX /** or AF_LOCAL */, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (sockfd < 0) {
        STREAM_ERROR << "failed to create unix domain socket socket ";
    }
    return sockfd;
}
int udsSockets::close(int sockfd) {
    return ::close(sockfd);
}

void udsSockets::bind(int fd, const std::string& path) {
    sockaddr_un unixAddr{};
    unixAddr.sun_family = AF_UNIX;
    std::strncpy(unixAddr.sun_path, path.c_str(), sizeof(unixAddr.sun_path) - 1);
    unlink(path.c_str());  /** 删除之前创建的套接字文件 */
    if (bind(fd, reinterpret_cast<const sockaddr*>(&unixAddr), socklen_t(sizeof(unixAddr))) == -1) {
        STREAM_ERROR << "failed to bind unix domain socket path";
    }
}

int udsSockets::connect(int fd, const std::string& path) {
    sockaddr_un clientAddress{};
    clientAddress.sun_family = AF_UNIX;
    strncpy(clientAddress.sun_path, path.c_str(), sizeof(clientAddress.sun_path) - 1);
    int ret = ::connect(fd, reinterpret_cast<const sockaddr*>(&clientAddress), socklen_t(sizeof(clientAddress)));
    if (ret == -1) {
        STREAM_ERROR << "failed to connect unix domain socket path";
        udsSockets::close(fd);
    }
    return ret;
}

void udsSockets::listen(int sockfd) {
    tcpSocket::listen(sockfd);
}

int udsSockets::accept(int fd, const std::string& path) {
    /** 已经bind了， accept不需要了地址了 */
    sockaddr_un serverAddress{};
    serverAddress.sun_family = AF_UNIX;
    strncpy(serverAddress.sun_path, path.c_str(), sizeof(serverAddress.sun_path) - 1);
    socklen_t clientAddressLength = sizeof(serverAddress);
    /** ------------------------------------------------------- */

    int connectedFd = ::accept4(fd, nullptr, nullptr, SOCK_NONBLOCK | SOCK_CLOEXEC);
    // fcntl(connectedFd, F_SETFD, FD_CLOEXEC); /**  设置为FD_CLOEXEC与非阻塞模式 */
    if (connectedFd == -1) {
        STREAM_ERROR << "failed to accept unix domain socket path";
        udsSockets::close(fd);
    }
    return connectedFd;
}

ssize_t udsSockets::read(int fd, void *buf, size_t count) {
    return ::read(fd, buf, count);
}


ssize_t udsSockets::write(int fd, const void *buf, size_t count) {
    STREAM_TRACE << "write data via unix domain socket";
    return ::write(fd, buf, count);
}