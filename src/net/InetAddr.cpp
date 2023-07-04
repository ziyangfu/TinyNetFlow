//
// Created by fzy on 23-5-15.
//

#include "InetAddr.h"
#include "../base/Logging.h"

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>  /** for gethostbyname_r*/
#include <string.h>
#include <cassert>

#include <thread>

/**
 * sockaddr_in:
 * struct sockaddr_in {
         sa_family_t    sin_family;  address family: AF_INET
         uint16_t       sin_port;    port in network byte order
         struct in_addr sin_addr;    internet address
     };
 * sockaddr_in6:
 *  struct sockaddr_in6 {
         sa_family_t     sin6_family;    address family: AF_INET6
         uint16_t        sin6_port;      port in network byte order
         uint32_t        sin6_flowinfo;  IPv6 flow information
         struct in6_addr sin6_addr;      IPv6 address
         uint32_t        sin6_scope_id;  IPv6 scope-id
     };
 */

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
        in_addr_t ip = loopbackOnly ? INADDR_LOOPBACK : INADDR_ANY;
        addr_.sin_addr.s_addr = htonl(ip);
        addr_.sin_port = htons(port);
    }
}

InetAddr::InetAddr(std::string ip, uint16_t port, bool ipv6) {
    const char* tempIp = const_cast<const char*>(ip.c_str());
    /** 当ip地址中有：时，把它当做ipv6处理 */
    if(ipv6 || strchr(ip.c_str(), ':')) {   /** fromIpPort() */
        memset(&addr6_, 0, sizeof addr6_);
        addr6_.sin6_family = AF_INET6;
        addr6_.sin6_port = htons(port);
        if(::inet_pton(AF_INET6, tempIp, &addr6_.sin6_addr) <= 0) {
            /** error */
        }
    }
    else {
        /** ipv4 */
        memset(&addr_, 0, sizeof addr_);
        addr_.sin_family = AF_INET;
        addr_.sin_port = htons(port);
        if(::inet_pton(AF_INET, tempIp, &addr_.sin_addr) <= 0) {
            /** error */
        }
    }
}

std::string InetAddr::toIp() const {
    char buf[64] = "";
    sockets::toIp(buf, sizeof buf, getSockAddr());
    return buf;
}

std::string InetAddr::toIpPort() const {
    char buf[64] = "";
    sockets::toIpPort(buf, sizeof buf, getSockAddr());
    return buf;
}


uint16_t InetAddr::getPort() const {
    return ::ntohs(addr_.sin_port);
}

void InetAddr::setScopeId(uint32_t scope_id) {
    if(getFamiliy() == AF_INET6) {
        addr6_.sin6_scope_id = scope_id;
    }
}

/*!
 * \brief 输入网站名，转换为InetAddr地址 */
thread_local char t_resolveBuffer[64 * 1024];
bool InetAddr::resolve(std::string hostname, netflow::net::InetAddr *result) {
    assert(result != NULL);
    struct hostent hent;
    struct hostent* he = NULL;
    int herrno = 0;
    memset(&hent, 0, sizeof hent);

    int ret = gethostbyname_r(hostname.c_str(), &hent, t_resolveBuffer, sizeof t_resolveBuffer, &he, &herrno);
    if (ret == 0 && he != NULL)
    {
        assert(he->h_addrtype == AF_INET && he->h_length == sizeof(uint32_t));
        result->addr_.sin_addr = *reinterpret_cast<struct in_addr*>(he->h_addr);
        return true;
    }
    else
    {
        if (ret)
        {
            STREAM_ERROR << "InetAddress::resolve";
        }
        return false;
    }
}