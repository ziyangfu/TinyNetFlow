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
 *      大端与小端转换
 * ----------------------------------------------------------------------------------------- */

#ifndef TINYNETFLOW_OSADAPTOR_ENDIAN_H
#define TINYNETFLOW_OSADAPTOR_ENDIAN_H

#include <cstdint>
#include <endian.h>

namespace netflow::osadaptor::net {
inline std::uint16_t hostToNetworkUint16(std::uint16_t host16) noexcept {
    return htobe16(host16);
}

inline std::uint32_t hostToNetworkUint32(std::uint32_t host32) noexcept {
    return htobe32(host32);
}

inline std::uint64_t hostToNetworkUint64(std::uint64_t host64) noexcept {
    return htobe64(host64);
}

inline std::uint16_t networkToHostUint16(std::uint16_t network16) noexcept {
    return be16toh(network16);
}

inline std::uint32_t networkToHostUint32(std::uint32_t network32) noexcept {
    return be32toh(network32);
}

inline std::uint64_t networkToHostUint64(std::uint64_t network64) noexcept {
    return be64toh(network64);
}

}  // namespace netflow::osadaptor::net

#endif //TINYNETFLOW_OSADAPTOR_ENDIAN_H
