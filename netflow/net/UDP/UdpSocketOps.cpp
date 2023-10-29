//
// Created by fzy on 23-8-24.
//

#include "UdpSocketOps.h"
#include "src/base/Logging.h"

using namespace netflow::net;
using namespace netflow::base;

int udpSockets::createUdpSocketV4() {
    int sockfd = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        STREAM_ERROR << "create UDP socket over IPv4 failed";
    }
    /** 设置地址复用
     * 设置端口复用
     * bind地址 */
    return sockfd;
}

int udpSockets::createUdpSocketV6() {
    int sockfd = ::socket(AF_INET6, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        STREAM_ERROR << "create UDP socket over IPv6 failed";
    }
    return sockfd;
}