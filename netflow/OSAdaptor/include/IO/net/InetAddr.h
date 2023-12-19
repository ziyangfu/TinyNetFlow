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
 *      地址统一描述，包括IPv4和IPv6两个部分, TCP及UDP两种协议
 * \file
 *      InetAddr.h
 * ----------------------------------------------------------------------------------------- */

#ifndef TINYNETFLOW_OSADAPTOR_INETADDR_H
#define TINYNETFLOW_OSADAPTOR_INETADDR_H

#include <netinet/in.h>
#include <string>

#include "IO/net/AddressCast.h"

namespace netflow::osadaptor::net {
/** 封装 sockaddr_in */
class InetAddr {
private:
    /** variant optional any C++17 */
    union  {
        struct sockaddr_in addr_;
        struct sockaddr_in6 addr6_;
    };
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

    std::string toIp() const;
    std::string toIpPort() const;
    uint16_t getPort() const;
    sa_family_t getFamiliy() const { return addr_.sin_family; }

    const struct sockaddr* getSockAddr() const { return sockaddrCast(&addr6_); }

    void setSockAddrInet6(const struct sockaddr_in6& addr6) { addr6_ = addr6; }
    void setScopeId(uint32_t scope_id);

    static bool resolve(std::string hostname, InetAddr* result);  /* FIXME: stringview */


    bool isMulticast() const noexcept;
    bool isLoopback() const noexcept;
    constexpr bool isIPv4() const noexcept;
    constexpr bool isIPv6() const noexcept;


};
} // namespace netflow::osadaptor::net



#endif //TINYNETFLOW_OSADAPTOR_INETADDR_H
