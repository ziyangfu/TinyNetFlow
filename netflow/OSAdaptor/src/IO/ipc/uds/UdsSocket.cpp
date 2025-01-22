//
// Created by fzy on 23-11-16.
//

#include "IO/ipc/uds/UdsSocket.h"
#include <spdlog/spdlog.h>
#include <cstring>

using namespace osadaptor::ipc;
/*!
 * \brief  创建一个非阻塞的流式unix域套接字
 * \details
 *      1. AF_UNIX 与 AF_LOCAL 同义
 *      2. 可以通过 SOCK_STREAM 创建流式套接字， 类似于TCP， 建立连接后使用read、write
 *      3. 可以通过 SOCK_DGRAM 创建报式套接字，类似于UDP, 后使用 sendto 与recvfrom
 *      4. SOCK_NONBLOCK设置为非阻塞模式，也可以默认设置为阻塞模式，然后写一个setBlocking函数，通过 fnctl 设置是否非阻塞模式
 *      5. SOCK_CLOEXEC是 close-on-exec机制，在fork子进程中执行exec的时候，会清理掉父进程创建的socket
 *      */
int udsSocket::createUdsSocket() {
    int sockfd = ::socket(AF_UNIX /** or AF_LOCAL */, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (sockfd < 0) {
        SPDLOG_ERROR("failed to create unix domain socket socket ");
    }
    return sockfd;
}
int udsSocket::close(int sockfd) {
    return ::close(sockfd);
}

void udsSocket::bind(int fd, const std::string& path) {
    sockaddr_un unixAddr{};
    unixAddr.sun_family = AF_UNIX;
    std::strncpy(unixAddr.sun_path, path.c_str(), sizeof(unixAddr.sun_path) - 1);
    unlink(path.c_str());  /** 删除之前创建的套接字文件 */
    if (bind(fd, reinterpret_cast<const sockaddr*>(&unixAddr), socklen_t(sizeof(unixAddr))) == -1) {
        SPDLOG_ERROR("failed to bind unix domain socket path");
    }
}

int udsSocket::connect(int fd, const std::string& path) {
    sockaddr_un clientAddress{};
    clientAddress.sun_family = AF_UNIX;
    strncpy(clientAddress.sun_path, path.c_str(), sizeof(clientAddress.sun_path) - 1);
    int ret = ::connect(fd, reinterpret_cast<const sockaddr*>(&clientAddress), socklen_t(sizeof(clientAddress)));
    if (ret == -1) {
        SPDLOG_ERROR("failed to connect unix domain socket path");
        udsSocket::close(fd);
    }
    return ret;
}

void udsSocket::listen(int sockfd) {
    int ret = ::listen(sockfd, SOMAXCONN);
    if(ret < 0){
        SPDLOG_ERROR("Failed to set socket fd to passive mode");
    }
}

int udsSocket::accept(int fd, const std::string& path) {
    /** 已经bind了， accept不需要了地址了 */
    sockaddr_un serverAddress{};
    serverAddress.sun_family = AF_UNIX;
    strncpy(serverAddress.sun_path, path.c_str(), sizeof(serverAddress.sun_path) - 1);
    socklen_t clientAddressLength = sizeof(serverAddress);
    /** ------------------------------------------------------- */

    int connectedFd = ::accept4(fd, nullptr, nullptr, SOCK_NONBLOCK | SOCK_CLOEXEC);
    // fcntl(connectedFd, F_SETFD, FD_CLOEXEC); /**  设置为FD_CLOEXEC与非阻塞模式 */
    if (connectedFd == -1) {
        SPDLOG_ERROR("failed to accept unix domain socket path");
        udsSocket::close(fd);
    }
    return connectedFd;
}

ssize_t udsSocket::read(int fd, void *buf, size_t count) {
    return ::read(fd, buf, count);
}

ssize_t udsSocket::write(int fd, const void *buf, size_t count) {
    SPDLOG_TRACE("write data via unix domain socket");
    return ::write(fd, buf, count);
}

/*!
 * \brief  调用sendmsg发送消息，可以发送memfd，用于shm，用于发送memfd时，message附带协议信息，包括shm的版本号，类型等
 * \details
            当向一个已关闭的套接字发送数据时，如果没有设置 MSG_NOSIGNAL 标志，系统默认会发送一个 SIGPIPE 信号
            给发送进程。这通常发生在尝试向一个已经关闭的套接字发送数据时，例如当接收方已经关闭连接或者进程突然终止。

            如果设置了 MSG_NOSIGNAL 标志，那么在遇到这种情况时，send() 或 sendto() 会返回 -1
            并设置 errno 为 EPIPE，而不是发送 SIGPIPE 信号。
 * */
std::size_t udsSocket::sendMsg(int udsFd, std::optional<int> memFd, std::string &message) {
    int const flags {MSG_NOSIGNAL};
    msghdr msg{};
    iovec iov{};
    msg.msg_flags = flags;
    iov.iov_base = const_cast<char*>(message.c_str());
    iov.iov_len = message.size();
    if (memFd.has_value()) {
        /** send memFd and message */
        char buf[CMSG_SPACE(sizeof(int))];
        cmsghdr* cmsg;
        msg.msg_name = nullptr;
        msg.msg_namelen = 0;
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;  /** fixme: 是这个吗？ */
        msg.msg_control = buf;
        msg.msg_controllen = sizeof(buf);

        cmsg = CMSG_FIRSTHDR(&msg);
        cmsg->cmsg_level = SOL_SOCKET;
        cmsg->cmsg_type = SCM_RIGHTS;
        cmsg->cmsg_len = CMSG_LEN(sizeof(int));
        *reinterpret_cast<int*>(CMSG_DATA(cmsg)) = memFd.value();
        msg.msg_controllen = cmsg->cmsg_len;
    }
    else {
        /** do nothing, only send message */
    }
    ssize_t sendBytes = ::sendmsg(udsFd, &msg, flags);
    if (sendBytes < 0) {
        SPDLOG_ERROR("failed to send message using syscall sendmsg via unix domain socket");
    }
    return static_cast<std::size_t>(sendBytes);
}

/*!
 * \brief  接收uds消息,调用recvmsg接收消息，接收到的消息放在recvMessage中
 *         当用于shm传递memfd时，recvMessage附带协议信息，包括shm的版本号，类型等
 * \return */
std::pair</** recvBytes*/std::size_t, /** memfd*/std::optional<int>>
                udsSocket::recvMsg(int udsFd, std::string &recvMessage) {
    std::optional<int> memfd;
    char recvBuffer[1024];
    int const flags {0};
    char buf[CMSG_SPACE(sizeof(int))];
    msghdr msg{};
    iovec iov{};
    cmsghdr* cmsg;

    /** 这里会附带一条shm的协议信息，包括version，type等信息 */
    /** 隐含了一次 char* --> void* */
    //iov.iov_base = const_cast<char*>(recvMessage.c_str());
    //iov.iov_len = recvMessage.size();
    iov.iov_base = recvBuffer;
    iov.iov_len = sizeof(recvBuffer);

    msg.msg_name = nullptr;
    msg.msg_namelen = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    msg.msg_control = buf;
    msg.msg_controllen = sizeof(buf);

    const ssize_t recvBytes = ::recvmsg(udsFd, &msg, flags);
    if (recvBytes < 0) {
        SPDLOG_ERROR("failed to receive message using syscall recvmsg via unix domain socket");
    }
    recvMessage = {recvBuffer, static_cast<size_t>(recvBytes)};
    SPDLOG_INFO("recvMessage is {}, size is {}, recvBytes is {}",
                        recvMessage, recvMessage.size(), recvBytes);
    if (recvMessage == "0521043") {
        cmsg = CMSG_FIRSTHDR(&msg);
        if (cmsg == nullptr || cmsg->cmsg_level != SOL_SOCKET || cmsg->cmsg_type != SCM_RIGHTS) {
            SPDLOG_ERROR("Invalid control message");
        }
        memfd = *reinterpret_cast<int*>(CMSG_DATA(cmsg));
    }
    return {static_cast<std::size_t>(recvBytes), memfd};
}