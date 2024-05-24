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
 *      地址统一描述
 * \file
 *      InetAddr.cpp
 * \details
      sockaddr_in:
        struct sockaddr_in {
             sa_family_t    sin_family;  address family: AF_INET
             uint16_t       sin_port;    port in network byte order
             struct in_addr sin_addr;    internet address
         };
      sockaddr_in6:
        struct sockaddr_in6 {
             sa_family_t     sin6_family;    address family: AF_INET6
             uint16_t        sin6_port;      port in network byte order
             uint32_t        sin6_flowinfo;  IPv6 flow information
             struct in6_addr sin6_addr;      IPv6 address
             uint32_t        sin6_scope_id;  IPv6 scope-id
         };
     sockaddr_un:
         struct sockaddr_un
          {
            __SOCKADDR_COMMON (sun_);
            char sun_path[108];		 Path name.
        };

 * ----------------------------------------------------------------------------------------- */

#include "IO/net/InetAddr.h"
#include "IO/net/Endian.h"
#include "IO/net/TcpSocket.h"
#include <spdlog/spdlog.h>

#include <arpa/inet.h>
#include <netdb.h>  /** for gethostbyname_r */
#include <cstring>
#include <cassert>
#include <algorithm>
#include <vector>

using namespace osadaptor::net;

InetAddr::InetAddr(uint16_t port, bool loopbackOnly, InetFamily family)
    : family_(family)
{
    if (family_ == InetFamily::kIPv6) {
        memset(&addr_, 0, sizeof addr_);
        std::get<sockaddr_in6>(addr_).sin6_family = AF_INET6;
        in6_addr ip = loopbackOnly ? in6addr_loopback : in6addr_any;  /** 回环或者监听所有地址 */
        std::get<sockaddr_in6>(addr_).sin6_addr = ip;
        std::get<sockaddr_in6>(addr_).sin6_port = hostToNetworkUint16(port);
    }
    else if (family_ == InetFamily::kIPv4){
        memset(&addr_, 0, sizeof addr_);
        std::get<sockaddr_in>(addr_).sin_family = AF_INET;
        in_addr_t ip = loopbackOnly ? INADDR_LOOPBACK : INADDR_ANY;
        std::get<sockaddr_in>(addr_).sin_addr.s_addr = hostToNetworkUint32(ip);
        std::get<sockaddr_in>(addr_).sin_port = hostToNetworkUint16(port);
    }
    else {
        SPDLOG_ERROR("unix domain socket need to a string path rather than ip and port");
    }
}

/*!
 * \brief 假设用户忘记设置 InetFamily,即默认IPv4 但是ip地址中却有：时，把它当做ipv6处理
 * */
InetAddr::InetAddr(std::string_view ip, uint16_t port, InetFamily family)
    : family_(family)
{
    const char* tempIp = ip.data();
    if(family_ == InetFamily::kIPv6 || strchr(tempIp, ':')) {
        memset(&addr_, 0, sizeof addr_);
        std::get<sockaddr_in6>(addr_).sin6_family = AF_INET6;
        std::get<sockaddr_in6>(addr_).sin6_port = hostToNetworkUint16(port);
        if(::inet_pton(AF_INET6, tempIp, &std::get<sockaddr_in6>(addr_).sin6_addr) <= 0) {
            SPDLOG_ERROR("failed to set IPv6 inet addr");
        }
    }
    else if (family_ == InetFamily::kIPv4){
        /** ipv4 */
        memset(&addr_, 0, sizeof addr_);
        std::get<sockaddr_in>(addr_).sin_family = AF_INET;
        std::get<sockaddr_in>(addr_).sin_port = hostToNetworkUint16(port);
        if(::inet_pton(AF_INET, tempIp, &std::get<sockaddr_in>(addr_).sin_addr) <= 0) {
            SPDLOG_ERROR("failed to set IPv4 inet addr");
        }
    }
    else {
        SPDLOG_ERROR("unix domain socket need to a string path rather than ip and port");
    }
}

InetAddr::InetAddr(const struct sockaddr_in &addr)
    : addr_(addr),
      family_(InetFamily::kIPv4)
{
}

InetAddr::InetAddr(const struct sockaddr_in6 &addr6)
        : addr_(addr6),
          family_(InetFamily::kIPv6)
{
}

InetAddr::InetAddr(const struct sockaddr_un &addr)
        : addr_(addr),
          family_(InetFamily::kUds)
{
}
InetAddr::InetAddr(std::string_view path) noexcept
    : family_(InetFamily::kUds)
{
    std::get<sockaddr_un>(addr_).sun_family = AF_UNIX;
    std::strncpy(std::get<sockaddr_un>(addr_).sun_path, path.data(),
                 sizeof(std::get<sockaddr_un>(addr_).sun_path) - 1);
}

bool InetAddr::operator==(const netflow::osadaptor::net::InetAddr &other) const noexcept {
    bool result = false;
    if (family_ == other.family_) {
        if (family_ == InetFamily::kIPv6) {
            result = std::equal(std::begin(std::get<sockaddr_in6>(addr_).sin6_addr.s6_addr),
                                std::end(std::get<sockaddr_in6>(addr_).sin6_addr.s6_addr),
                                std::begin(std::get<sockaddr_in6>(other.addr_).sin6_addr.s6_addr));
        }
        else if (family_ == InetFamily::kIPv4) {
            result = (std::get<sockaddr_in>(addr_).sin_addr.s_addr
                       == std::get<sockaddr_in>(other.addr_).sin_addr.s_addr);

        }
        else {
            result = std::equal(std::begin(std::get<sockaddr_un>(addr_).sun_path),
                       std::end(std::get<sockaddr_un>(addr_).sun_path),
                       std::begin(std::get<sockaddr_un>(other.addr_).sun_path));
        }
    }
    else {
        result = false;
    }
    return result;
}

std::string InetAddr::toStringIp() const {
    char buf[64] = "";
    tcpSocket::toIp(buf, sizeof buf, getSockAddr());
    return buf;
}

std::string InetAddr::toStringIpPort() const {
    char buf[64] = "";
    tcpSocket::toIpPort(buf, sizeof buf, getSockAddr());
    return buf;
}

sa_family_t InetAddr::getInetFamily() const {
    if (family_ == InetFamily::kIPv6) {
        return std::get<sockaddr_in6>(addr_).sin6_family;
    }
    else if (family_ == InetFamily::kIPv4) {
        return std::get<sockaddr_in>(addr_).sin_family;
    }
    else {
        return std::get<sockaddr_un>(addr_).sun_family;
    }
}

InetAddr::InetFamily InetAddr::getFamily() const {
    return family_;
}

uint16_t InetAddr::getPort() const {
    uint16_t port{0};
    if (family_ == InetFamily::kIPv6) {
        port = std::get<sockaddr_in6>(addr_).sin6_port;
    }
    else if (family_ == InetFamily::kIPv4) {
        port = std::get<sockaddr_in>(addr_).sin_port;
    }
    else {
        SPDLOG_ERROR("unix domain socket have not port");
    }
    return port;
}

void InetAddr::setScopeId(uint32_t scope_id) {
    if(family_ == InetFamily::kIPv6) {
        std::get<sockaddr_in6>(addr_).sin6_scope_id = scope_id;
    }
    else {
        SPDLOG_ERROR("IPv4 and unix domain socket do not support scope_id");
    }
}

/*!
 * \brief 输入网站名，转换为InetAddr地址， 暂时保留
 * */
thread_local char t_resolveBuffer[64 * 1024];
bool InetAddr::resolve(std::string_view hostname, InetAddr *result) {
    assert(result != nullptr);
    struct hostent hent;
    struct hostent* he = nullptr;
    int herrno = 0;
    memset(&hent, 0, sizeof hent);

    int ret = gethostbyname_r(hostname.data(), &hent, t_resolveBuffer, sizeof t_resolveBuffer, &he, &herrno);
    if (ret == 0 && he != nullptr)
    {
        assert(he->h_addrtype == AF_INET && he->h_length == sizeof(uint32_t));
        std::get<sockaddr_in>(result->addr_).sin_addr = *reinterpret_cast<struct in_addr*>(he->h_addr);
        return true;
    }
    else
    {
        if (ret)
        {
            SPDLOG_ERROR("InetAddress::resolve");
        }
        return false;
    }
}

const struct sockaddr *InetAddr::getSockAddr() const {
    if (family_ == InetFamily::kIPv6) {
        return sockaddrCast(&std::get<sockaddr_in6>(addr_));
    }
    else if (family_ == InetFamily::kIPv4) {
        return sockaddrCast(&std::get<sockaddr_in>(addr_));
    }
    else if (family_ == InetFamily::kUds) {
        return sockaddrCast(&std::get<sockaddr_un>(addr_));
    }
}


bool InetAddr::isIPv4() const noexcept {
    if (family_ == InetFamily::kIPv4) {
        return true;
    }
    else {
        return false;
    }
}

bool InetAddr::isIPv6() const noexcept {
    if (family_ == InetFamily::kIPv6) {
        return true;
    }
    else {
        return false;
    }
}
/*!
 * \brief 判断地址是否是多播地址
 * \details
 *      如果是IPv6,  多播地址是 FF：XX:XX:...， 因此判断第一个字节是否为 FF
 *      如果是IPv4， 多播地址是 224.X.X.X， 因此判断前8位是否为224
 *      如果是UDS，   unix domain socket不支持多播
 *      */
bool InetAddr::isMulticast() const noexcept {
    bool result = false;
    if (family_ == InetFamily::kIPv6) {
        result = std::get<sockaddr_in6>(addr_).sin6_addr.s6_addr[0] == static_cast<std::uint8_t>(0xFF);
    }
    else if (family_ == InetFamily::kIPv4) {
        result = (networkToHostUint32(std::get<sockaddr_in>(addr_).sin_addr.s_addr) & 0xF0000000)
                 == 0xE0000000;
    }
    else {
        SPDLOG_ERROR("unix domain socket do not support multicast");
    }
    return result;
}
/*!
 * \brief 检测地址是否是回环地址
 * \details
 *        ::1 是 IPv6 回环地址
 *        127.0.0.0/8 是 IPv4 回环地址的地址段
 * */
bool InetAddr::isLoopBack() const noexcept {
    bool result = false;
    if (family_ == InetFamily::kIPv6) {
        std::vector<std::uint8_t> zeroBytes;
        for (int i = 0; i < 15; i++) {
            zeroBytes.push_back(std::get<sockaddr_in6>(addr_).sin6_addr.s6_addr[i]);
        }
        /** 前15个字节均为0，且第16个字节为0x01 */
        result = std::all_of(zeroBytes.begin(), zeroBytes.end(),
                 [](const std::uint8_t current) -> bool {
                        return current == static_cast<std::uint8_t>(0x00);})
            && std::get<sockaddr_in6>(addr_).sin6_addr.s6_addr[15] == static_cast<std::uint8_t>(0x01);

    }
    else if (family_ == InetFamily::kIPv4) {
        uint32_t address = networkToHostUint32(std::get<sockaddr_in>(addr_).sin_addr.s_addr);
        result = (address & 0xFF000000) == 0x7F000000;
    }
    else {
        SPDLOG_ERROR("unix domain socket do not support multicast");
    }
    return result;
}