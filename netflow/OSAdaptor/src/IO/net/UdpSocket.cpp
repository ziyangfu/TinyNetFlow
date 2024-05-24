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
 *      UDP socket接口
 * \file
 *      UdpSocket.cpp
 * ----------------------------------------------------------------------------------------- */

#include "IO/net/UdpSocket.h"
#include <spdlog/spdlog.h>
#include <sys/ioctl.h>

using namespace osadaptor::net;
/*!
 * \details
 *      SOCK_NONBLOCK ： 设置为非阻塞模式
 *      SOCK_CLOEXEC  ： 在执行新程序时，该套接字会自动关闭，不会被新程序继承和使用
 * */
int udpSocket::createNoBlockingUdpSocket(sa_family_t family) {
    int sockfd = ::socket(family, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (sockfd < 0) {
        SPDLOG_ERROR("create UDP socket  failed, family is {}", family);
    }
    return sockfd;
}

int udpSocket::createBlockingUdpSocket(sa_family_t family) {
    int sockfd = ::socket(family, SOCK_DGRAM | SOCK_CLOEXEC, 0);
    if (sockfd < 0) {
        SPDLOG_ERROR("create UDP socket  failed, family is {}", family);
    }
    return sockfd;
}

/*!
 * \brief 演示用ioctl也可以设置非阻塞套接字，实际上方默认常见的UDP socket即是非阻塞的
 * */
void udpSocket::setBlockingMode(int fd, bool noblock) {
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

void udpSocket::bind(int fd, const struct sockaddr* addr) {
    int ret = ::bind(fd, addr, sizeof(struct sockaddr));
    if (ret < 0) {
        SPDLOG_ERROR("Failed to bind UDP local addr");
    }
}

int udpSocket::connect(int fd, const struct sockaddr* addr) {
    int ret = ::connect(fd, addr, sizeof(struct sockaddr));
    return ret;
}

/*!
 * \brief 当未调用connect函数时，需要选择sendTo直接发送
 * */
ssize_t udpSocket::sendTo(int fd, const struct sockaddr *addr, const void *data, size_t length) {
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
ssize_t udpSocket::recvFrom(int fd, char* buf, size_t length, sockaddr* addr) {
    socklen_t addrLength = sizeof(sockaddr);
    ssize_t readN = ::recvfrom(fd, buf, length, 0, addr, &addrLength);
    return readN;
}

void udpSocket::send(int fd, const char* data, size_t length) {
    ssize_t sendLength = ::send(fd, data, length, 0);
    //return static_cast<size_t>(sendLength) == length;
}

ssize_t udpSocket::read(int fd, void *buf, size_t count) {
    return ::read(fd, buf, count);
}

ssize_t udpSocket::write(int fd, const void *buf, size_t count) {
    return ::write(fd, buf,count);
}

int udpSocket::close(int fd) {
    return ::close(fd);
}

void udpSocket::setUdpReuseAddr(int sockfd, bool on) {
    int opt = on ? 1 : 0;
    if (::setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
                     (char*)&opt, socklen_t(sizeof(opt))) < 0) {
        SPDLOG_ERROR("Failed to set UDP reuse addr");
    }
}

void udpSocket::setUdpReusePort(int sockfd, bool on) {
    int opt = on ? 1 : 0;
    if (::setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT,
                     (char*)&opt, socklen_t(sizeof(opt))) < 0) {
        SPDLOG_ERROR("Failed to set UDP reuse port");
    }
}

void udpSocket::setReceiveBufferSize(int sockfd, int size) {
    if (::setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size)) == -1) {
        SPDLOG_ERROR("failed to set UDP receive buffer");
    }
}

void udpSocket::setSendBufferSize(int sockfd, int size) {
    if (::setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size)) == -1) {
        SPDLOG_ERROR("failed to set UDP send  buffer");
    }
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
void udpSocket::setPriority(int sockfd, int32_t priority) {
    if (::setsockopt(sockfd, SOL_SOCKET, SO_PRIORITY, &priority, sizeof(priority)) == -1) {
        SPDLOG_ERROR("failed to set TCP priority");
    }
}

void udpSocket::setUnicastTtl(int sockfd, uint8_t hops) {
    if (::setsockopt(sockfd, IPPROTO_IP, IP_TTL, &hops, sizeof(hops)) == -1) {
        SPDLOG_ERROR("failed to set IP TTL");
    }
}

/** -------------------------------- 多播部分 --------------------------------------------------------*/
bool udpSocket::joinMulticastGroupV4(int sockfd, const sockaddr_in* addr) {
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

bool udpSocket::joinMulticastGroupV6(int sockfd, const sockaddr_in6* addr6) {
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

bool udpSocket::leaveMulticastGroupV4(int sockfd, const sockaddr_in* addr) {
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

bool udpSocket::leaveMulticastGroupV6(int sockfd, const sockaddr_in6* addr6) {
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

void udpSocket::setMulticastTtlV4(int sockfd, int ttl) {
    if (setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0) {
        SPDLOG_ERROR("Failed to set IPv4 TTL");
    }
}

void udpSocket::setMulticastTtlV6(int sockfd, int ttl) {
    if (setsockopt(sockfd, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &ttl, sizeof(ttl)) < 0) {
        SPDLOG_ERROR("Failed to set IPv6 TTL");
    }
}

void udpSocket::setMulticastNetworkInterfaceV4(int sockfd, const sockaddr_in* addr) {
    //struct in_addr addr;
    //if (inet_pton(AF_INET, ip.c_str(), &(addr.s_addr)) == 0) {
    //    STREAM_ERROR << "Invalid IPv4 address: " << ip ;
    //}
    if (::setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_IF,
                     (char*)&addr, sizeof(addr)) < 0) {
        SPDLOG_ERROR("Failed to set IPv4 IP_MULTICAST_I");
    }
}

void udpSocket::setMulticastNetworkInterfaceV6(int sockfd, const sockaddr_in6* addr6) {
    if (::setsockopt(sockfd, IPPROTO_IPV6, IPV6_MULTICAST_IF,
                     (char*)addr6, sizeof(*addr6)) < 0) {
        SPDLOG_ERROR("Failed to set IPv6 IP_MULTICAST_IF");
    }
}

void udpSocket::setMulticastLoopV4(int sockfd, bool on) {
    int loopFlag = on ?  1 : 0;
    if (setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_LOOP,
                   (char*)&loopFlag, sizeof(loopFlag)) < 0) {
        SPDLOG_ERROR("Failed to set IPv4 IP_MULTICAST_LOOP");
    }
}

void udpSocket::setMulticastLoopV6(int sockfd, bool on) {
    int loopFlag = on ?  1 : 0;
    if (setsockopt(sockfd, IPPROTO_IPV6, IPV6_MULTICAST_LOOP,
                   (char*)&loopFlag, sizeof(loopFlag)) < 0) {
        SPDLOG_ERROR("Failed to set IPv6 IPV6_MULTICAST_LOOP");
    }
}
/** -------------------------------- 多播部分结束 --------------------------------------------------------*/

/** -------------------------------- 广播 ---------------------------------------------------------------*/

int udpSocket::setBroadcast(int fd, bool on) {
    int opt = on ? 1 : 0;
    int ret = setsockopt(fd, SOL_SOCKET, SO_BROADCAST,
                         (char*)&opt, static_cast<socklen_t>(sizeof(opt)));
    if (ret == -1) {
        SPDLOG_ERROR("Failed to set UDP broadcast");
    }
    return ret;
}