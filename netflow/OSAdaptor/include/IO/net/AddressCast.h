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
 *      地址转换，包括sockaddr，sockaddr_in， sockaddr_in6等
 * ----------------------------------------------------------------------------------------- */

#ifndef TINYNETFLOW_OSADAPTOR_ADDRESSCAST_H
#define TINYNETFLOW_OSADAPTOR_ADDRESSCAST_H

namespace netflow::osadaptor::net {

namespace detail {
/*!
 * \brief static_cast允许向上和向下转型，而implicit_cast只允许向上转型
 * */
template<typename To, typename From>
inline To implicit_cast(From const &f)
{
    return f;
}
} // namespace detail

/** 函数重载
 * sockaddr, sockaddr_in, sockaddr_in6 的转换函数 */
inline const struct sockaddr* sockaddrCast(const struct sockaddr_in* addr) {
    return static_cast<const struct sockaddr*>(detail::implicit_cast<const void*>(addr));
}
/** sockaddr_in6 to sockaddr */
inline const struct sockaddr* sockaddrCast(const struct sockaddr_in6* addr) {
    return static_cast<const struct sockaddr*>(detail::implicit_cast<const void*>(addr));
}
inline struct sockaddr* sockaddrCast(struct sockaddr_in6* addr) {
    return static_cast<struct sockaddr*>(detail::implicit_cast<void*>(addr));
}
/** sockaddr to sockaddr_in */
inline const struct sockaddr_in* sockaddrInCast(const struct sockaddr* addr) {
    return static_cast<const struct sockaddr_in*>(detail::implicit_cast<const void*>(addr));
}
/** sockaddr to sockaddr_in6 */
inline const struct sockaddr_in6* sockaddrIn6Cast(const struct sockaddr* addr) {
    return static_cast<const struct sockaddr_in6*>(detail::implicit_cast<const void*>(addr));
}

}  // namespace netflow::osadaptor::net

#endif //TINYNETFLOW_OSADAPTOR_ADDRESSCAST_H
