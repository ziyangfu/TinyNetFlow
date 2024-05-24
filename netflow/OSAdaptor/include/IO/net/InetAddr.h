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
 *      地址统一描述，包括IPv4和IPv6两个部分, 涉及TCP、UDP、UDS三种协议
 * \file
 *      InetAddr.h
 * ----------------------------------------------------------------------------------------- */

#ifndef TINYNETFLOW_OSADAPTOR_INETADDR_H
#define TINYNETFLOW_OSADAPTOR_INETADDR_H

#include <netinet/in.h>
#include <sys/un.h>
#include <string>
#include <variant>      /** C++17 */
#include <string_view>
#include "IO/net/AddressCast.h"

namespace osadaptor::net {

class InetAddr {
public:
    enum class InetFamily : uint8_t {
        kIPv4,
        kIPv6,
        kUds
    };
private:
    std::variant<sockaddr_in, sockaddr_in6, sockaddr_un> addr_;
    InetFamily family_;
public:
    /*!
     * \brief for only ip, 一般用于 TCP server 监听所有地址
     * */
    explicit InetAddr(uint16_t port = 0, bool loopbackOnly = false, InetFamily family = InetFamily::kIPv4);
    /*!
     * \brief for ip + port
     * */
    explicit InetAddr(std::string_view ip, uint16_t port, InetFamily family = InetFamily::kIPv4);
    /*!
     * \brief for IPv4 sockaddr_in
     * */
    explicit InetAddr(const struct sockaddr_in& addr);
    /*!
     * \brief for IPv6 sockaddr_in6
     * */
    explicit InetAddr(const struct sockaddr_in6& addr6);
    /*!
    * \brief for unix domain socket， sockaddr_un
    * */
    explicit InetAddr(const struct sockaddr_un& addr);
    /*!
     * \brief for unix domain socket
     * */
    explicit InetAddr(std::string_view path) noexcept;

    InetAddr(const InetAddr& other) noexcept = default;
    InetAddr& operator=(const InetAddr& other) noexcept = default;
    InetAddr(InetAddr&& other) noexcept = default;
    InetAddr& operator=(InetAddr&& other) noexcept = default;
    ~InetAddr() = default;

    bool operator==(const InetAddr& other) const noexcept;

    std::string toStringIp() const;
    std::string toStringIpPort() const;
    uint16_t getPort() const;
    sa_family_t getInetFamily() const;
    InetFamily getFamily() const;
    const struct sockaddr* getSockAddr() const;

    void setSockAddrInet6(const struct sockaddr_in6& addr6) { addr_ = addr6; }
    void setScopeId(uint32_t scope_id);
    static bool resolve(std::string_view hostname, InetAddr* result);  /** 输入网站名，转换为InetAddr地址 */

    bool isMulticast() const noexcept;
    bool isLoopBack() const noexcept;
    bool isIPv4() const noexcept;
    bool isIPv6() const noexcept;

};
} // namespace osadaptor::net

#endif //TINYNETFLOW_OSADAPTOR_INETADDR_H
