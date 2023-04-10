//
// Created by fzy on 23-3-13.
//

#include "inet_address.h"
#include "../base/logging.h"
#include "endian.h"
#include "socket_ops.h"
/*!
 * 这两个头文件 <netdb.h> 和 <netinet/in.h> 是在C语言中用于网络编程的头文件，通常在使用套接字(Socket)编程时需要包含这两个头文件。

<netdb.h>：该头文件定义了一些网络编程中用于访问和解析网络主机信息的函数和数据结构，包括获取主机名和IP地址之间的转换、获取服务名和端口号
 之间的转换、获取本机主机信息等。其中常用的函数包括 gethostbyname()、gethostbyaddr()、getservbyname()、getservbyport() 等。

<netinet/in.h>：该头文件定义了一些网络编程中用于处理网络协议的数据结构和常量，包括 IPv4 和 IPv6 地址的定义、套接字地址结构的定义、
 网络字节序和主机字节序之间的转换函数、IP协议头的定义等。其中常用的数据结构包括 struct sockaddr_in 和 struct sockaddr_in6 等。

这两个头文件通常在网络编程中一起使用，以便使用其中定义的函数和数据结构来进行网络通信、地址解析、协议处理等操作。在使用这些头文件时，
 需要注意其对应的函数和数据结构的使用方式和语法，遵循网络编程的规范和标准。
 */
#include <netdb.h>
#include <netinet/in.h>


// INADDR_ANY use (type)value casting.
#pragma GCC diagnostic ignored "-Wold-style-cast"
static const in_addr_t kInaddrAny = INADDR_ANY;
static const in_addr_t kInaddrLoopback = INADDR_LOOPBACK;
#pragma GCC diagnostic error "-Wold-style-cast"

//     /* Structure describing an Internet socket address.  */
//     struct sockaddr_in {
//         sa_family_t    sin_family; /* address family: AF_INET */
//         uint16_t       sin_port;   /* port in network byte order */
//         struct in_addr sin_addr;   /* internet address */
//     };

//     /* Internet address. */
//     typedef uint32_t in_addr_t;
//     struct in_addr {
//         in_addr_t       s_addr;     /* address in network byte order */
//     };

//     struct sockaddr_in6 {
//         sa_family_t     sin6_family;   /* address family: AF_INET6 */
//         uint16_t        sin6_port;     /* port in network byte order */
//         uint32_t        sin6_flowinfo; /* IPv6 flow information */
//         struct in6_addr sin6_addr;     /* IPv6 address */
//         uint32_t        sin6_scope_id; /* IPv6 scope-id */
//     };

using namespace muduo;
using namespace muduo::net;

static_assert(sizeof(InetAddress) == sizeof(struct sockaddr_in6),
              "InetAddress is same size as sockaddr_in6");
static_assert(offsetof(sockaddr_in, sin_family) == 0, "sin_family offset 0");
static_assert(offsetof(sockaddr_in6, sin6_family) == 0, "sin6_family offset 0");
static_assert(offsetof(sockaddr_in, sin_port) == 2, "sin_port offset 2");
static_assert(offsetof(sockaddr_in6, sin6_port) == 2, "sin6_port offset 2");

InetAddress::InetAddress(uint16_t portArg, bool loopbackOnly, bool ipv6)
{
    static_assert(offsetof(InetAddress, addr6_) == 0, "addr6_ offset 0");
    static_assert(offsetof(InetAddress, addr_) == 0, "addr_ offset 0");
    if (ipv6)
    {
        memZero(&addr6_, sizeof addr6_);
        addr6_.sin6_family = AF_INET6;
        in6_addr ip = loopbackOnly ? in6addr_loopback : in6addr_any;
        addr6_.sin6_addr = ip;
        addr6_.sin6_port = sockets::hostToNetwork16(portArg);
    }
    else
    {
        memZero(&addr_, sizeof addr_);
        addr_.sin_family = AF_INET;
        in_addr_t ip = loopbackOnly ? kInaddrLoopback : kInaddrAny;
        addr_.sin_addr.s_addr = sockets::hostToNetwork32(ip);
        addr_.sin_port = sockets::hostToNetwork16(portArg);
    }
}

InetAddress::InetAddress(StringArg ip, uint16_t portArg, bool ipv6)
{
    if (ipv6 || strchr(ip.c_str(), ':'))
    {
        memZero(&addr6_, sizeof addr6_);
        sockets::fromIpPort(ip.c_str(), portArg, &addr6_);
    }
    else
    {
        memZero(&addr_, sizeof addr_);
        sockets::fromIpPort(ip.c_str(), portArg, &addr_);
    }
}

string InetAddress::toIpPort() const
{
    char buf[64] = "";
    sockets::toIpPort(buf, sizeof buf, getSockAddr());
    return buf;
}

string InetAddress::toIp() const
{
    char buf[64] = "";
    sockets::toIp(buf, sizeof buf, getSockAddr());
    return buf;
}

uint32_t InetAddress::ipv4NetEndian() const
{
    assert(family() == AF_INET);
    return addr_.sin_addr.s_addr;
}

uint16_t InetAddress::port() const
{
    return sockets::networkToHost16(portNetEndian());
}

static __thread char t_resolveBuffer[64 * 1024];

bool InetAddress::resolve(StringArg hostname, InetAddress* out)
{
    assert(out != NULL);
    struct hostent hent;
    struct hostent* he = NULL;
    int herrno = 0;
    memZero(&hent, sizeof(hent));

    int ret = gethostbyname_r(hostname.c_str(), &hent, t_resolveBuffer, sizeof t_resolveBuffer, &he, &herrno);
    if (ret == 0 && he != NULL)
    {
        assert(he->h_addrtype == AF_INET && he->h_length == sizeof(uint32_t));
        out->addr_.sin_addr = *reinterpret_cast<struct in_addr*>(he->h_addr);
        return true;
    }
    else
    {
        if (ret)
        {
            LOG_SYSERR << "InetAddress::resolve";
        }
        return false;
    }
}

void InetAddress::setScopeId(uint32_t scope_id)
{
    if (family() == AF_INET6)
    {
        addr6_.sin6_scope_id = scope_id;
    }
}
