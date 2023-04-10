//
// Created by fzy on 23-3-13.
//

#include "socket.h"

#include "../base/logging.h"
#include "inet_address.h"
#include "socket_ops.h"

#include <netinet/in.h>
#include <netinet/tcp.h>

#include <stdio.h>

using namespace muduo;
using namespace muduo::net;

Socket::~Socket() {
    sockets::close(sockfd_);
}

bool Socket::getTcpInfo(struct tcp_info* tcpi) const {
    socklen_t len = sizeof(*tcpi);
    memZero(tcpi, len);  /*! ../base/types.h */
    return ::getsockopt(sockfd_, SOL_TCP, TCP_INFO, tcpi, &len);
}
/*! \brief 以字符串的形式输出tcp_info信息
 * \arg buf : 保存 tcp_info */
bool Socket::getTcpInfoString(char *buf, int len) const {
    struct tcp_info tcpi;
    bool ok = getTcpInfo(&tcpi);
    if (ok) {
        snprintf(buf, len, "unrecovered=%u "
                           "rto=%u ato=%u snd_mss=%u rcv_mss=%u "
                           "lost=%u retrans=%u rtt=%u rttvar=%u "
                           "sshthresh=%u cwnd=%u total_retrans=%u",
                 tcpi.tcpi_retransmits,  // Number of unrecovered [RTO] timeouts
                 tcpi.tcpi_rto,          // Retransmit timeout in usec
                 tcpi.tcpi_ato,          // Predicted tick of soft clock in usec
                 tcpi.tcpi_snd_mss,
                 tcpi.tcpi_rcv_mss,
                 tcpi.tcpi_lost,         // Lost packets
                 tcpi.tcpi_retrans,      // Retransmitted packets out
                 tcpi.tcpi_rtt,          // Smoothed round trip time in usec
                 tcpi.tcpi_rttvar,       // Medium deviation
                 tcpi.tcpi_snd_ssthresh,
                 tcpi.tcpi_snd_cwnd,
                 tcpi.tcpi_total_retrans);  // Total retransmits for entire connection
    }
    return ok;
}

void Socket::bindAddress(const muduo::net::InetAddress &addr) {
    sockets::bindOrDie(sockfd_, addr.getSockAddr());
}

void Socket::listen() {
    sockets::listenOrDie(sockfd_);
}

int Socket::accept(muduo::net::InetAddress *peeraddr) {
    struct sockaddr_in6 addr;  /*! ipv6 */
    memZero(&addr, sizeof addr);
    int connfd = sockets::accept(sockfd_, &addr);
    if (connfd >= 0) {
        peeraddr->setSockAddrInet6(addr);
    }
    return connfd;
}

void Socket::shutdownWrite() {
    sockets::shutdownWrite(sockfd_);
}

void Socket::setTcpNoDelay(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY,
                 &optval, static_cast<socklen_t>(sizeof optval));
}

void Socket::setReuseAddr(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR,
                 &optval, static_cast<socklen_t>(sizeof optval));

}

void Socket::setReusePort(bool on) {
#ifdef SO_REUSEPORT
    int optval = on ? 1 : 0;
    int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT,
                           &optval, static_cast<socklen_t>(sizeof optval));
    if (ret < 0 && on) {
        LOG_SYSERR << "SO_REUSEPORT failed.";
    }
#else
    if (on) {
        LOG_ERROR << "SO_REUSEPORT is not supported.";
    }
#endif
}

/**
 * int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);
 *  sockfd: 套接字文件描述符，用于标识要设置选项的套接字。
    level: 选项所属的协议层，可以是SOL_SOCKET（通用套接字选项）或特定协议的协议层，例如IPPROTO_TCP（TCP协议选项）。
    optname: 要设置的选项名，具体取值依赖于level的设置，例如SO_REUSEADDR表示重用地址选项。
    optval: 指向存放选项值的缓冲区的指针。
    optlen: 缓冲区长度。

    setsockopt函数用于设置套接字选项的值。常用的选项包括：
    SO_REUSEADDR：允许地址重用，通常在服务器程序中使用。
    SO_KEEPALIVE：启用TCP的keep-alive机制，检测连接是否活跃。
    SO_LINGER：设置套接字的linger选项，控制套接字关闭行为。
    SO_RCVBUF、SO_SNDBUF：设置接收和发送缓冲区的大小。
    TCP_NODELAY：禁用Nagle算法，提高小数据包传输的效率。

    setsockopt函数的返回值为0表示成功，-1表示失败，失败时可以通过errno变量获取具体的错误信息。
    */
void Socket::setKeepAlive(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval,
                 static_cast<socklen_t>(sizeof optval));
}