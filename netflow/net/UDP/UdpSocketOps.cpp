//
// Created by fzy on 23-8-24.
//

#include "UdpSocketOps.h"
#include "netflow/base/Logging.h"

using namespace netflow::net;
using namespace netflow::base;

int udpSockets::createUdpSocketV4() {
    int sockfd = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0) {
        STREAM_ERROR << "create UDP socket over IPv4 failed";
    }
    return sockfd;
}

int udpSockets::createUdpSocketV6() {
    int sockfd = ::socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0) {
        STREAM_ERROR << "create UDP socket over IPv6 failed";
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

bool udpSockets::joinMulticastGroupV4(int sockfd, const std::string& ip) {
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(ip.c_str());
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);  /** 这表示什么？ */
    if (::setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) <  0) {
        STREAM_ERROR << "Failed to join IPv4 multicast group";
        return false;
    }
    return true;
}

bool udpSockets::joinMulticastGroupV6(int sockfd, const std::string &ip6) {
    struct ipv6_mreq mreq6;
    inet_pton(AF_INET6, ip6.c_str(), &(mreq6.ipv6mr_multiaddr));
    mreq6.ipv6mr_interface = 0;
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
