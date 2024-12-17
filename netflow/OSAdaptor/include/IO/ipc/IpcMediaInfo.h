/** ----------------------------------------------------------------------------------------
 * \copyright
 * Copyright (c) 2024 by the TinyNetFlow project authors. All Rights Reserved.
 *
 * This file is open source software, licensed to you under the ter；ms
 * of the Apache License, Version 2.0 (the "License").  See the NOTICE file
 * distributed with this work for additional information regarding copyright
 * ownership.  You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 * -----------------------------------------------------------------------------------------
 * \brief
 *      osadaptor IPC 通用定义， IPC对外支持 unix domian sockt 与 shared memory 两种方式
 * \file
 *      IpcMediaAddr.h
 * ----------------------------------------------------------------------------------------- */

#ifndef OSADAPTOR_IO_IPC_IPCMEDIAADDR_H
#define OSADAPTOR_IO_IPC_IPCMEDIAADDR_H

#include <cstdint>

namespace osadaptor::ipc {
/*!
 * \brief 定义 uds 与 shared memory 的统一地址 ， 采用 domain与port来描述
 * */
struct UnixDomainPath {
    int domain;
    int port;
};

/*!
 * \brief IPC的两种方式
 * */
enum class IpcType : std::uint8_t {
    kUds,
    kShm
};
/*!
 * \brief  默认的 IPC 方式为 uds
 * */
IpcType kDefaultIpcType {IpcType::kUds};

} // namespace osadaptor::ipc


#endif // OSADAPTOR_IO_IPC_IPCMEDIAADDR_H
