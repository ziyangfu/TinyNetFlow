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
 *      OS socket接口封装，POSIX socket API
 * \file
 *      OsSocketInterface.cpp
 * ----------------------------------------------------------------------------------------- */

#include "IO/net/OsSocketInterface.h"
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
#include <sys/ioctl.h>

using namespace osadaptor::net;
/*!
 * \brief 创建非阻塞socket
 * \details
 *      SOCK_NONBLOCK ： 设置为非阻塞模式
 *      SOCK_CLOEXEC  ： 在执行新程序时，该套接字会自动关闭，不会被新程序继承和使用
 * */
int socketInterface::createNonblockingSocket(sa_family_t family, int32_t type) noexcept {
    int sockfd = ::socket(family, type | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if(sockfd < 0) {
        SPDLOG_ERROR("failed to create nonblocking socket");
    }
    return sockfd;
}

int socketInterface::createBlockingSocket(SocketFamily family, SocketType type) {
    int sockfd = ::socket(family.value, type.value | SOCK_CLOEXEC, 0);
    if(sockfd < 0) {
        SPDLOG_ERROR("failed to create nonblocking socket");
    }
    return sockfd;
}

/*!
 * \brief 是否设置阻塞模式
 * \details
 *      采用fcntl与ioctl均可以设置非阻塞套接字
 * */
void socketInterface::setBlockingMode(int fd, bool noblock) {
#if 0
    /** 设置非阻塞的方式之一： 采用 fcntl */
    int flags {::fcntl(fd, F_GETFL, 0) };
    if (flags == -1) {
        /** error */
    }
    else {
        if (noblock) {
            flags = flags & ~O_NONBLOCK;
        }
        else {
            flags = flags | O_NONBLOCK;
        }
    }
    if (::fcntl(fd, F_SETFL, flags) == -1) {
        /** error */
    }
#endif
    /** 设置非阻塞的方式之二： 采用 ioctl */
    int ul = noblock;
    int ret = ::ioctl(fd, FIONBIO, &ul);
    if (ret == -1) {
        SPDLOG_ERROR("Failed to set UDP socket noblock mode");
    }
}

int socketInterface::connect(int sockfd, const struct sockaddr* addr){
    int ret = ::connect(sockfd, addr, sizeof(struct sockaddr));
    if (ret < 0) {
        SPDLOG_ERROR("Failed to connect addr");
    }
    return ret;
}

void socketInterface::bind(int sockfd, const struct sockaddr* addr){
    int ret = ::bind(sockfd, addr, sizeof(struct sockaddr));
    if(ret < 0){
        SPDLOG_ERROR("Failed to bind local addr");
    }
}

void socketInterface::listen(int sockfd, std::int32_t backlog){
    int ret {-1};
    if (backlog < SOMAXCONN) {
        ret = ::listen(sockfd, backlog);
    }
    else {
        ret = ::listen(sockfd, SOMAXCONN);
    }
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
int socketInterface::accept(int sockfd, struct sockaddr_in6* addr){
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

ssize_t socketInterface::read(int sockfd, void* buf, size_t count){
    return ::read(sockfd, buf, count);

}

/*!
 * \brief 允许单个系统调用读入或写出自一个或多个缓冲区
 * */
ssize_t socketInterface::readv(int sockfd, const struct iovec* iov, int iovcnt){
    return ::readv(sockfd, iov, iovcnt);

}

ssize_t socketInterface::write(int sockfd, const void* buf, size_t count){
    return ::write(sockfd, buf, count);
}
/*!
 * \brief 当未调用connect函数时，需要选择sendTo直接发送
 * */
ssize_t socketInterface::sendTo(int fd, const struct sockaddr *addr, const void *data, size_t length) {
    if (length == 0) {
        SPDLOG_WARN("length is 0, please check");
    }
    ssize_t sendNum = ::sendto(fd, data, length, 0, addr, sizeof(*addr));
    if (static_cast<size_t>(sendNum) != length) {
        SPDLOG_ERROR("send data is not complete");
    }
    return sendNum;
}

/*!
 * \brief 接收， 与sendTo匹配
 * */
ssize_t socketInterface::recvFrom(int fd, char* buf, size_t length, sockaddr* addr) {
    socklen_t addrLength = sizeof(sockaddr);
    ssize_t readN = ::recvfrom(fd, buf, length, 0, addr, &addrLength);
    return readN;
}


int socketInterface::close(int sockfd) noexcept{
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
void socketInterface::shutdownWrite(int sockfd){
    ::shutdown(sockfd, SHUT_WR);
}

struct sockaddr_in6 socketInterface::getLocalAddr(int sockfd) {
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
struct sockaddr_in6 socketInterface::getPeerAddr(int sockfd) {
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
bool socketInterface::isSelfConnect(int sockfd){
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

int socketInterface::getSocketError(int sockfd) {
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
void socketInterface::toIpPort(char* buf, size_t size,
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
void socketInterface::toIp(char* buf, size_t size,
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
void socketInterface::fromIpPort(const char* ip, uint16_t port,
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
void socketInterface::fromIpPort(const char* ip, uint16_t port,
                         struct sockaddr_in6* addr)
{
    addr->sin6_family = AF_INET6;
    addr->sin6_port = htons(port);
    if (::inet_pton(AF_INET6, ip, &addr->sin6_addr) <= 0)
    {
        SPDLOG_ERROR("error in IPv6 ::inet_pton");
    }
}

void socketInterface::setTcpNoDelay(int sockfd, bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY,
                 &optval, static_cast<socklen_t>(sizeof optval));
}

void socketInterface::setReuseAddr(int sockfd, bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
                 &optval, static_cast<socklen_t>(sizeof optval));
}

void socketInterface::setReusePort(int sockfd, bool on) {
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

void socketInterface::setKeepAlive(int sockfd, bool on) {
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
void socketInterface::setPriority(int sockfd, int32_t priority) {
    if (::setsockopt(sockfd, SOL_SOCKET, SO_PRIORITY, &priority, sizeof(priority)) == -1) {
        SPDLOG_ERROR("failed to set TCP priority");
    }
}

void socketInterface::setUnicastTtl(int sockfd, uint8_t hops) {
    if (::setsockopt(sockfd, IPPROTO_IP, IP_TTL, &hops, sizeof(hops)) == -1) {
        SPDLOG_ERROR("failed to set IP TTL");
    }
}

void socketInterface::setReceiveBufferSize(int sockfd, int size) {
    if (::setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size)) == -1) {
        SPDLOG_ERROR("failed to set UDP receive buffer");
    }
}

void socketInterface::setSendBufferSize(int sockfd, int size) {
    if (::setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size)) == -1) {
        SPDLOG_ERROR("failed to set UDP send  buffer");
    }
}

/** -------------------------------- 多播部分 --------------------------------------------------------*/
bool socketInterface::joinMulticastGroupV4(int sockfd, const sockaddr_in* addr) {
    struct ip_mreq mreq;
    mreq.imr_multiaddr = addr->sin_addr; /** 多播组地址 */
    // mreq.imr_multiaddr.s_addr = inet_addr(ip.c_str());
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);  /** 本地的IP地址 */
    if (::setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) <  0) {
        SPDLOG_ERROR("Failed to join IPv4 multicast group");
        return false;
    }
    return true;
}

bool socketInterface::joinMulticastGroupV6(int sockfd, const sockaddr_in6* addr6) {
    struct ipv6_mreq mreq6;
    // const std::string &ip6
    //inet_pton(AF_INET6, ip6.c_str(), &(mreq6.ipv6mr_multiaddr)); /** IPv6 多播地址 */
    mreq6.ipv6mr_multiaddr = addr6->sin6_addr;
    mreq6.ipv6mr_interface = 0; /** IPv6 本地地址 */
    if (::setsockopt(sockfd, IPPROTO_IPV6, IPV6_JOIN_GROUP,
                     &mreq6, sizeof(mreq6)) <  0) {
        SPDLOG_ERROR("Failed to join IPv6 multicast group");
        return false;
    }
    return true;
}

bool socketInterface::leaveMulticastGroupV4(int sockfd, const sockaddr_in* addr) {
    struct ip_mreq mreq;
    //mreq.imr_multiaddr.s_addr = inet_addr(ip.c_str()); // ip is std::string
    mreq.imr_multiaddr = addr->sin_addr;
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if (setsockopt(sockfd, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        SPDLOG_ERROR("Failed to leave IPv6 multicast group");
        return false;
    }
    return true;
}

bool socketInterface::leaveMulticastGroupV6(int sockfd, const sockaddr_in6* addr6) {
    struct ipv6_mreq mreq6;
    //inet_pton(AF_INET6, ip6.c_str(), &(mreq6.ipv6mr_multiaddr));
    mreq6.ipv6mr_multiaddr = addr6->sin6_addr;
    mreq6.ipv6mr_interface = 0;
    if (setsockopt(sockfd, IPPROTO_IPV6, IPV6_LEAVE_GROUP, &mreq6, sizeof(mreq6)) < 0) {
        SPDLOG_ERROR("Failed to leave IPv6 multicast group");
        return false;
    }
    return true;
}

void socketInterface::setMulticastTtlV4(int sockfd, int ttl) {
    if (setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0) {
        SPDLOG_ERROR("Failed to set IPv4 TTL");
    }
}

void socketInterface::setMulticastTtlV6(int sockfd, int ttl) {
    if (setsockopt(sockfd, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &ttl, sizeof(ttl)) < 0) {
        SPDLOG_ERROR("Failed to set IPv6 TTL");
    }
}

void socketInterface::setMulticastNetworkInterfaceV4(int sockfd, const sockaddr_in* addr) {
    //struct in_addr addr;
    //if (inet_pton(AF_INET, ip.c_str(), &(addr.s_addr)) == 0) {
    //    STREAM_ERROR << "Invalid IPv4 address: " << ip ;
    //}
    if (::setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_IF,
                     (char*)&addr, sizeof(addr)) < 0) {
        SPDLOG_ERROR("Failed to set IPv4 IP_MULTICAST_I");
    }
}

void socketInterface::setMulticastNetworkInterfaceV6(int sockfd, const sockaddr_in6* addr6) {
    if (::setsockopt(sockfd, IPPROTO_IPV6, IPV6_MULTICAST_IF,
                     (char*)addr6, sizeof(*addr6)) < 0) {
        SPDLOG_ERROR("Failed to set IPv6 IP_MULTICAST_IF");
    }
}

void socketInterface::setMulticastLoopV4(int sockfd, bool on) {
    int loopFlag = on ?  1 : 0;
    if (setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_LOOP,
                   (char*)&loopFlag, sizeof(loopFlag)) < 0) {
        SPDLOG_ERROR("Failed to set IPv4 IP_MULTICAST_LOOP");
    }
}

void socketInterface::setMulticastLoopV6(int sockfd, bool on) {
    int loopFlag = on ?  1 : 0;
    if (setsockopt(sockfd, IPPROTO_IPV6, IPV6_MULTICAST_LOOP,
                   (char*)&loopFlag, sizeof(loopFlag)) < 0) {
        SPDLOG_ERROR("Failed to set IPv6 IPV6_MULTICAST_LOOP");
    }
}
/** -------------------------------- 多播部分结束 --------------------------------------------------------*/

/** -------------------------------- 广播 ---------------------------------------------------------------*/

int socketInterface::setBroadcast(int fd, bool on) {
    int opt = on ? 1 : 0;
    int ret = setsockopt(fd, SOL_SOCKET, SO_BROADCAST,
                         (char*)&opt, static_cast<socklen_t>(sizeof(opt)));
    if (ret == -1) {
        SPDLOG_ERROR("Failed to set UDP broadcast");
    }
    return ret;
}
