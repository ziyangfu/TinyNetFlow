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
 *      mmap共享内存预定义参数
 * \file
 *      ShmConstant.h
 * ----------------------------------------------------------------------------------------- */

#ifndef OSADAPTOR_IO_IPC_SHM_SHMCONSTANT_H
#define OSADAPTOR_IO_IPC_SHM_SHMCONSTANT_H

#include <string>
#include "IO/ipc/IpcMediaAddr.h"



namespace osadaptor::ipc {

namespace shm {
const std::string kDefaultShmDirectory {"/tmp"};
const std::string kShmDomainStr{"osadaptor_shm_domain_"};
const std::string kShmPortStr{"_port_"};
const std::string kDefaultSharedMemoryPath {"/tmp/osadaptor_shm_domain_10_port_10"};

std::string kShmPathFormat {"%s/osadaptor_shm_domain_%u_port_%u"};

#include <string_view>
std::string_view formatPath {"%s/osadaptor_shm_domain_%u_port_%u"_sv};

std::string kClientFormat {"XXX_domain_%u_port_%u_pid_%u_count_%u"};

constexpr std::uint32_t kDefaultShmFileSize { 64 * 1024 }; /** 编译器计算，默认64K bytes */

}  // namespace shm

} // namespace osadaptor::ipc

#endif // OSADAPTOR_IO_IPC_SHM_SHMCONSTANT_H
