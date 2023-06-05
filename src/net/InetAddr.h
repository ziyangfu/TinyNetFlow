//
// Created by fzy on 23-5-15.
//

#ifndef TINYNETFLOW_INETADDR_H
#define TINYNETFLOW_INETADDR_H

#include <netinet/in.h>
#include <string>

#include "SocketsOps.h"

namespace netflow::net {
/** 封装 sockaddr_in */
class InetAddr {
public:
    /** for only ip, 一般用于 TCP server 监听所有地址 */
    explicit InetAddr(uint16_t port = 0, bool loopbackOnly = false, bool ipv6 = false);
    /** for ip + port */
    InetAddr(std::string ip, uint16_t port, bool ipv6 = false);
    /** for IPv4 sockaddr_in */
    explicit InetAddr(const struct sockaddr_in& addr)
            : addr_(addr)
    {
    }
    /** for IPv6 sockaddr_in6 */
    explicit InetAddr(const struct sockaddr_in6& addr6)
            : addr6_(addr6)
    {
    }

    std::string sockaddrToStringIp() const;
    std::string sockaddrToStringIpPort() const;
    uint16_t getPort() const;
    sa_family_t getFamiliy() const { return addr_.sin_family; }

    const struct sockaddr* getSockAddr() const { return sockets::sockaddr_in_to_sockaddr(&addr_); }

    void setSockAddrInet6(const struct sockaddr_in6& addr6) { addr6_ = addr6; }
    void setScopeId(uint32_t scope_id);

private:
    union  {
        struct sockaddr_in addr_;
        struct sockaddr_in6 addr6_;
    };
};
} // namespace netflow::net::sockets



#endif //TINYNETFLOW_INETADDR_H
