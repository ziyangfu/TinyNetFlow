//
// Created by fzy on 23-8-24.
//

#include "UdpSocketOps.h"
#include "netflow/base/Logging.h"

#include <sys/ioctl.h>

using namespace netflow::net;
using namespace netflow::base;
/*!
 * \details
 *      SOCK_NONBLOCK ： 设置为非阻塞模式
 *      SOCK_CLOEXEC  ： 在执行新程序时，该套接字会自动关闭，不会被新程序继承和使用
 * */
int udpSockets::createUdpSocket(sa_family_t family) {
    int sockfd = ::socket(family, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (sockfd < 0) {
            STREAM_ERROR << "create UDP socket  failed, family is " << family;
    }
    return sockfd;
}

void udpSockets::setUdpReuseAddr(int sockfd, bool on) {
    if (::setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
        STREAM_ERROR << "Failed to set UDP reuse addr";
    }
}

void udpSockets::setUdpReusePort(int sockfd, bool on) {
    if (::setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on)) < 0) {
        STREAM_ERROR << "Failed to set UDP reuse port";
    }
}

/** 演示用ioctl也可以设置非阻塞套接字，实际上方默认常见的UDP socket即是非阻塞的 */
int udpSockets::setNoBlocked(int fd, bool noblock) {
    int ul = noblock;
    int ret = ::ioctl(fd, FIONBIO, &ul);
    if (ret == -1) {
        STREAM_ERROR << "Failed to set UDP socket noblock mode";
    }
    return ret;
}


void udpSockets::bind(int fd, const struct sockaddr* addr) {
    int ret = ::bind(fd, addr, sizeof(struct sockaddr));
    if (ret < 0) {
        STREAM_ERROR << "Failed to bind UDP local addr";
    }
}

int udpSockets::createAndBindSocket(const struct sockaddr *addr, bool isUseV6) {
    int fd = createUdpSocket(isUseV6);
    bind(fd, addr);
    return fd;
}

int udpSockets::connect(int fd, const struct sockaddr* addr) {
    int ret = ::connect(fd, addr, sizeof(struct sockaddr));
    return ret;
}

/** 直接发送，未调用connect函数 */
bool udpSockets::sendTo(int fd, const struct sockaddr *addr, const char *data, size_t length) {
    if (length == 0) {
        return false;
    }
    int sendNum = ::sendto(fd, data, length, 0, addr, sizeof(*addr));
    if (sendNum != length) {
        return false;
    }
    return true;
}

bool udpSockets::sendTo(int fd, const struct sockaddr *addr, const std::string &message) {
    sendTo(fd, addr, message.c_str(), message.length());
}

bool udpSockets::send(int fd, const char* data, size_t length) {
    int sendLength = ::send(fd, data, length, 0);
    return static_cast<size_t>(sendLength) == length;
}

ssize_t udpSockets::read(int fd, void *buf, size_t count) {
    return ::read(fd, buf, count);
}

ssize_t udpSockets::write(int fd, void *buf, size_t count) {
    return ::write(fd, buf,count);
}

/** 接收， 与sendTo匹配 */
int udpSockets::recvFrom(int fd, void* buf, size_t length, sockaddr* addr) {
    socklen_t addrLength = sizeof(sockaddr);
    int readN = recvfrom(fd, buf, length, 0, addr, &addrLength);
    return readN;
}

int udpSockets::close(int fd) {
    return ::close(fd);
}

/** -------------------------------- 多播部分 --------------------------------------------------------*/
bool udpSockets::joinMulticastGroupV4(int sockfd, const struct sockaddr_in* addr) {
    struct ip_mreq mreq;
    mreq.imr_multiaddr = addr->sin_addr; /** 多播组地址 */
    // mreq.imr_multiaddr.s_addr = inet_addr(ip.c_str());
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);  /** 本地的IP地址 */
    if (::setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) <  0) {
        STREAM_ERROR << "Failed to join IPv4 multicast group";
        return false;
    }
    return true;
}

bool udpSockets::joinMulticastGroupV6(int sockfd, const std::string &ip6) {
    struct ipv6_mreq mreq6;
    inet_pton(AF_INET6, ip6.c_str(), &(mreq6.ipv6mr_multiaddr)); /** IPv6 多播地址 */
    mreq6.ipv6mr_interface = 0; /** IPv6 本地地址 */
    if (::setsockopt(sockfd, IPPROTO_IPV6, IPV6_JOIN_GROUP,
                     &mreq6, sizeof(mreq6)) <  0) {
        STREAM_ERROR << "Failed to join IPv6 multicast group";
        return false;
    }
    return true;
}

bool udpSockets::leaveMulticastGroupV4(int sockfd, const std::string &ip) {
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(ip.c_str());
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if (setsockopt(sockfd, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        STREAM_ERROR << "Failed to leave IPv6 multicast group";
        return false;
    }
    return true;
}

bool udpSockets::leaveMulticastGroupV6(int sockfd, const std::string &ip6) {
    struct ipv6_mreq mreq6;
    inet_pton(AF_INET6, ip6.c_str(), &(mreq6.ipv6mr_multiaddr));
    mreq6.ipv6mr_interface = 0;
    if (setsockopt(sockfd, IPPROTO_IPV6, IPV6_LEAVE_GROUP, &mreq6, sizeof(mreq6)) < 0) {
        STREAM_ERROR << "Failed to leave IPv6 multicast group";
        return false;
    }
    return true;
}

void udpSockets::setMulticastTtlV4(int sockfd, int ttl) {
    if (setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0) {
        STREAM_ERROR << "Failed to set IPv4 TTL";
    }
}

void udpSockets::setMulticastTtlV6(int sockfd, int ttl) {
    if (setsockopt(sockfd, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &ttl, sizeof(ttl)) < 0) {
        STREAM_ERROR << "Failed to set IPv6 TTL";
    }
}

void udpSockets::setMulticastNetworkInterfaceV4(int sockfd, const std::string &ip) {
    struct in_addr addr;
    if (inet_pton(AF_INET, ip.c_str(), &(addr.s_addr)) == 0) {
        STREAM_ERROR << "Invalid IPv4 address: " << ip ;
    }
    if (::setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_IF,
                     (char*)&addr, sizeof(addr)) < 0) {
        STREAM_ERROR << "Failed to set IPv4 IP_MULTICAST_IF";
    }
}

void udpSockets::setMulticastNetworkInterfaceV6(int sockfd, const sockaddr_in6* addr6) {
    if (::setsockopt(sockfd, IPPROTO_IPV6, IPV6_MULTICAST_IF,
                     (char*)addr6, sizeof(*addr6)) < 0) {
        STREAM_ERROR << "Failed to set IPv6 IP_MULTICAST_IF";
    }
}

void udpSockets::setMulticastLoopV4(int sockfd, bool on) {
    int loopFlag = on ?  1 : 0;
    if (setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_LOOP,
                   (char*)&loopFlag, sizeof(loopFlag)) < 0) {
        STREAM_ERROR << "Failed to set IPv4 IP_MULTICAST_LOOP";
    }
}

void udpSockets::setMulticastLoopV6(int sockfd, bool on) {
    int loopFlag = on ?  1 : 0;
    if (setsockopt(sockfd, IPPROTO_IPV6, IPV6_MULTICAST_LOOP,
                   (char*)&loopFlag, sizeof(loopFlag)) < 0) {
        STREAM_ERROR << "Failed to set IPv6 IPV6_MULTICAST_LOOP";
    }
}
/** -------------------------------- 多播部分结束 --------------------------------------------------------*/

/** -------------------------------- 广播 ---------------------------------------------------------------*/

int udpSockets::setBroadcast(int fd, bool on) {
    int opt = on ? 1 : 0;
    int ret = setsockopt(fd, SOL_SOCKET, SO_BROADCAST,
                         (char*)&opt, static_cast<socklen_t>(sizeof(opt)));
    if (ret == -1) {
        STREAM_ERROR << "Failed to set UDP broadcast";
    }
    return ret;
}