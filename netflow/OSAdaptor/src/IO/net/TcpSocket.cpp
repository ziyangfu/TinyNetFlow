/** ----------------------------------------------------------------------------------------
 * \copyright
 * Copyright (c) 2023 by the TinyNetFlow project authors. All Rights Reserved.
 *
 * This file is open source software, licensed to you under the ter；ms
 * of the Apache License, Version 2.0 (the "License").  See the NOTICE file
 * distributed with this work for additional information regarding copyright
 * ownership.  You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 * -----------------------------------------------------------------------------------------
 * \brief
 *      TCP socket接口封装，POSIX socket API
 * \file
 *      TcpSocket.cpp
 * ----------------------------------------------------------------------------------------- */

#include "IO/net/TcpSocket.h"
#include "IO/net/AddressCast.h"

#include <spdlog/spdlog.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/uio.h>  /** for readv */

#include <unistd.h>
#include <strings.h> /** bzero */
#include <cstring> /** memcmp */
#include <cassert>
#include <cerrno>

using namespace netflow::osadaptor::net;

int tcpSocket::createNonblockingSocket(sa_family_t family){
    int sockfd = ::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if(sockfd < 0) {
        SPDLOG_ERROR("failed to create nonblocking TCP socket");
    }
    return sockfd;
}

int tcpSocket::createBlockingSocket(sa_family_t family) {
    int sockfd = ::socket(family, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if(sockfd < 0) {
        SPDLOG_ERROR("failed to create nonblocking TCP socket");
    }
    return sockfd;
}

void tcpSocket::setBlockingMode(int sockfd, bool noblock) {
    /** 设置非阻塞的方式之一： 采用 fcntl */
    int flags {::fcntl(sockfd, F_GETFL, 0) };
    if (flags == -1) {
        SPDLOG_ERROR("failed to fcntl setting");
    }
    else {
        if (noblock) {
            flags = flags & ~O_NONBLOCK;
        }
        else {
            flags = flags | O_NONBLOCK;
        }
    }
    if (::fcntl(sockfd, F_SETFL, flags) == -1) {
        SPDLOG_ERROR("failed to fcntl setting");
    }
}

int tcpSocket::connect(int sockfd, const struct sockaddr* addr){
    int ret = ::connect(sockfd, addr, sizeof(struct sockaddr));
    return ret;
}

void tcpSocket::bind(int sockfd, const struct sockaddr* addr){
    int ret = ::bind(sockfd, addr, sizeof(struct sockaddr));
    if(ret < 0){
        SPDLOG_ERROR("Failed to bind TCP local addr");
    }
}

void tcpSocket::listen(int sockfd){
    int ret = ::listen(sockfd, SOMAXCONN);
    if(ret < 0){
        SPDLOG_ERROR("Failed to set socket fd to passive mode");
    }
}

/*!
 * \details accept4函数可以直接将返回的socket设置为非阻塞的。
 *          只要将 accept4() 函数最后一个参数 flags 设置成 SOCK_NONBLOCK 即可
 *          否则，则需要写成这样：
                socklen_t addrlen = sizeof(clientaddr);
                int clientfd = accept(listenfd, &clientaddr, &addrlen);
                if (clientfd != -1)
                {
                    int oldSocketFlag = fcntl(clientfd, F_GETFL, 0);
                    int newSocketFlag = oldSocketFlag | O_NONBLOCK;
                    fcntl(clientfd, F_SETFL,  newSocketFlag);
                }
 *          */
int tcpSocket::accept(int sockfd, struct sockaddr_in6* addr){
    socklen_t addrlen = static_cast<socklen_t>(sizeof *addr);
    int connfd = ::accept4(sockfd, sockaddrCast(addr), &addrlen,
                           SOCK_NONBLOCK | SOCK_CLOEXEC);
    if(connfd < 0){
        int savedErrno = errno;
        switch (savedErrno)
        {
            case EAGAIN:
            case ECONNABORTED:
            case EINTR:
            case EPROTO: // ???
            case EPERM:
            case EMFILE: // per-process lmit of open file desctiptor ???
                // expected errors
                errno = savedErrno;
                break;
            case EBADF:
            case EFAULT:
            case EINVAL:
            case ENFILE:
            case ENOBUFS:
            case ENOMEM:
            case ENOTSOCK:
            case EOPNOTSUPP:
                SPDLOG_ERROR("accept system call : unexpected error, {}", savedErrno);
                break;
            default:
                SPDLOG_ERROR("accept system call : unknown error, {}", savedErrno);
                break;
        }
    }
    return connfd;
}

ssize_t tcpSocket::read(int sockfd, void* buf, size_t count){
    return ::read(sockfd, buf, count);

}

/*!
 * \brief 允许单个系统调用读入或写出自一个或多个缓冲区
 * */
ssize_t tcpSocket::readv(int sockfd, const struct iovec* iov, int iovcnt){
    return ::readv(sockfd, iov, iovcnt);

}

ssize_t tcpSocket::write(int sockfd, const void* buf, size_t count){
    return ::write(sockfd, buf, count);
}

int tcpSocket::close(int sockfd){
    return ::close(sockfd);
}
/*!
 * \brief 单独关闭写功能
 * \details
 *         ::shutdown :
               Shut down all or part of the connection open on socket FD.
               HOW determines what to shut down:
                 SHUT_RD   = No more receptions;
                 SHUT_WR   = No more transmissions;
                 SHUT_RDWR = No more receptions or transmissions.
               Returns 0 on success, -1 for errors.
*/
void tcpSocket::shutdownWrite(int sockfd){
    ::shutdown(sockfd, SHUT_WR);
}

struct sockaddr_in6 tcpSocket::getLocalAddr(int sockfd) {
    struct sockaddr_in6 localaddr;
    bzero(&localaddr, sizeof localaddr);
    socklen_t addrlen = static_cast<socklen_t>(sizeof localaddr);
    if (::getsockname(sockfd, sockaddrCast(&localaddr), &addrlen) < 0) {
        SPDLOG_ERROR("failed to get sock name");
    }
    return localaddr;
}

/*!
 * \brief 通过socket描述符，获取sockaddr_in6 地址
 * */
struct sockaddr_in6 tcpSocket::getPeerAddr(int sockfd) {
    struct sockaddr_in6 peeraddr;
    bzero(&peeraddr, sizeof peeraddr);
    socklen_t addrlen = static_cast<socklen_t>(sizeof peeraddr);
    if (::getpeername(sockfd, sockaddrCast(&peeraddr), &addrlen) < 0) {
        SPDLOG_ERROR("failed to get peer name");
    }
    return peeraddr;
}

/*!
 * \brief 检测是否是自连接
 * \details 自连接即 源 IP，port 与 目标 IP port 完全一致
 * 出现场景：本地IPC通信
 * 表现形式：源 IP，port 与 目标 IP port 完全一致
 * 原因分析：
 *      1. 当socket 调用connect()时,os会为该socket分配一个被称作临时端口的源port作为bind,
 *          分配策略就是os维护一个计数,每次有进程申请就自增;
        2. 分配完临时端口后的socket,开始向目标端口发送SYN,因为都在本机,如果这个临时端口和目标端口恰好一致了,
        就会导致端口自己向自己发送SYN,从而触发两个active socket同时连接的处理逻辑,而os内部并没有对源ip:port
        和目的ip:port进行判断,所以TCP自连接发生了
  解决办法：
    1. 设置合适的临时端口分配段,保证不会和目标端口重叠
        vim  /etc/sysctl.conf
        net.ipv4.ip_local_port_range=1024  65535
    2. 在程序中判断（即本方法）
*/
bool tcpSocket::isSelfConnect(int sockfd){
    struct sockaddr_in6 localaddr = getLocalAddr(sockfd);
    struct sockaddr_in6 peeraddr = getPeerAddr(sockfd);
    if(localaddr.sin6_family == AF_INET) {
        /** reinterpret_cast 运算符并不会改变括号中运算对象的值，而是对该对象从位模式上进行重新解释 */
        const sockaddr_in* laddr4 = reinterpret_cast<struct sockaddr_in*>(&localaddr);
        const sockaddr_in* paddr4 = reinterpret_cast<struct sockaddr_in*>(&peeraddr);
        return laddr4->sin_port == paddr4->sin_port && laddr4->sin_addr.s_addr == paddr4->sin_addr.s_addr;
    }
    else if(localaddr.sin6_family == AF_INET6) {
        return localaddr.sin6_port == peeraddr.sin6_port &&
                (memcmp(&localaddr.sin6_addr, &peeraddr.sin6_addr, sizeof localaddr.sin6_addr) == 0);
    }
    else {
        return false;
    }
}

int tcpSocket::getSocketError(int sockfd) {
    int optval;
    socklen_t optlen = static_cast<socklen_t>(sizeof optval);
    if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
        return errno;
    }
    else {
        return optval;
    }
}

/*!
 * \brief 将sockaddr转换为字符串格式的IP地址与端口
 * */
void tcpSocket::toIpPort(char* buf, size_t size,
                       const struct sockaddr* addr)
{
    if (addr->sa_family == AF_INET6)
    {
        buf[0] = '[';
        toIp(buf+1, size-1, addr);
        size_t end = ::strlen(buf);
        const struct sockaddr_in6* addr6 = sockaddrIn6Cast(addr);
        uint16_t port = ntohs(addr6->sin6_port);
        assert(size > end);
        snprintf(buf+end, size-end, "]:%u", port);
        return;
    }
    /** IPv4 */
    toIp(buf, size, addr);
    size_t end = ::strlen(buf);
    const struct sockaddr_in* addr4 = sockaddrInCast(addr);
    uint16_t port = ntohs(addr4->sin_port);
    assert(size > end);
    snprintf(buf+end, size-end, ":%u", port);
}

/*!
 * \brief 将sockaddr转换为字符串格式的IP地址
 * */
void tcpSocket::toIp(char* buf, size_t size,
                   const struct sockaddr* addr)
{
    if (addr->sa_family == AF_INET)
    {
        assert(size >= INET_ADDRSTRLEN);  /** INET_ADDRSTRLEN 16 */
        const struct sockaddr_in* addr4 = sockaddrInCast(addr);
        ::inet_ntop(AF_INET, &addr4->sin_addr, buf, static_cast<socklen_t>(size));
    }
    else if (addr->sa_family == AF_INET6)
    {
        assert(size >= INET6_ADDRSTRLEN); /** INET6_ADDRSTRLEN 46 */
        const struct sockaddr_in6* addr6 = sockaddrIn6Cast(addr);
        ::inet_ntop(AF_INET6, &addr6->sin6_addr, buf, static_cast<socklen_t>(size));
    }
}

/*!
 * \brief 将字符串格式的IP地址与端口转换为sockaddr_in格式， IPv4
 * */
void tcpSocket::fromIpPort(const char* ip, uint16_t port,
                         struct sockaddr_in* addr)
{
    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);
    if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0)
    {
        SPDLOG_ERROR("error in IPv4 ::inet_pton");
    }
}

/*!
 * \brief 将字符串格式的IP地址与端口转换为sockaddr_in格式， IPv6
 * */
void tcpSocket::fromIpPort(const char* ip, uint16_t port,
                         struct sockaddr_in6* addr)
{
    addr->sin6_family = AF_INET6;
    addr->sin6_port = htons(port);
    if (::inet_pton(AF_INET6, ip, &addr->sin6_addr) <= 0)
    {
        SPDLOG_ERROR("error in IPv6 ::inet_pton");
    }
}

void tcpSocket::setTcpNoDelay(int sockfd, bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY,
                 &optval, static_cast<socklen_t>(sizeof optval));
}

void tcpSocket::setReuseAddr(int sockfd, bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
                 &optval, static_cast<socklen_t>(sizeof optval));
}

void tcpSocket::setReusePort(int sockfd, bool on) {
#ifdef SO_REUSEPORT
    int optval = on ? 1 : 0;
    int ret = ::setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT,
                           &optval, static_cast<socklen_t>(sizeof optval));
    if (ret < 0 && on)
    {
        SPDLOG_ERROR("SO_REUSEPORT failed");
    }
#else
    if (on)
    {
        SPDLOG_ERROR("SO_REUSEPORT is not supported");
    }
#endif
}

void tcpSocket::setKeepAlive(int sockfd, bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE,
                 &optval, static_cast<socklen_t>(sizeof optval));
}

/*!
 * \details
    Linux kernel 5.10:
        #define TC_PRIO_BESTEFFORT		0
        #define TC_PRIO_FILLER			1
        #define TC_PRIO_BULK			2
        #define TC_PRIO_INTERACTIVE_BULK	4
        #define TC_PRIO_INTERACTIVE		6
        #define TC_PRIO_CONTROL			7

        #define TC_PRIO_MAX			15
*/
void tcpSocket::setPriority(int sockfd, int32_t priority) {
    if (::setsockopt(sockfd, SOL_SOCKET, SO_PRIORITY, &priority, sizeof(priority)) == -1) {
        SPDLOG_ERROR("failed to set TCP priority");
    }
}

void tcpSocket::setUnicastTtl(int sockfd, uint8_t hops) {
    if (::setsockopt(sockfd, IPPROTO_IP, IP_TTL, &hops, sizeof(hops)) == -1) {
        SPDLOG_ERROR("failed to set IP TTL");
    }
}
