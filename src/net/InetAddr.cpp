//
// Created by fzy on 23-5-15.
//

#include "InetAddr.h"
#include <netinet/in.h>
#include <netdb.h>  /** for gethostbyname_r*/
#include <string.h>

using namespace netflow::net;

InetAddr::InetAddr(uint16_t port, bool loopbackOnly, bool ipv6) {
    if (ipv6) {
        memset(&addr6_, 0, sizeof addr6_);
        addr6_.sin6_family = AF_INET6;
        in6_addr ip = loopbackOnly ? in6addr_loopback : in6addr_any;  /** 回环或者监听所有地址 */
        addr6_.sin6_addr = ip;
        addr6_.sin6_port = htons(port);
    }
    else {
        memset(&addr_, 0, sizeof addr_);
        addr_.sin_family = AF_INET;
        in_addr ip = loopbackOnly ? INADDR_LOOPBACK : INADDR_ANY;
        addr_.sin_addr.s_addr = htonl(ip);
        addr_.sin_port = htons(port);
    }
}

InetAddr::InetAddr(std::string ip, uint16_t port, bool ipv6) {
    if(ipv6) {
        memset(&addr6_, 0, sizeof addr6_);
        addr6_.sin6_family = AF_INET6;

    }
    else {
        memset(&addr_, 0, sizeof addr_);
        addr_.sin_family = AF_INET;

        addr_.sin_addr = htonl();
        addr_.sin_port = htons(port);

    }
}