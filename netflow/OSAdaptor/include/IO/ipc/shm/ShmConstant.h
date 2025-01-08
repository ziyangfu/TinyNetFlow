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

#ifndef OSADAPTOR_IO_IPC_SHM__SHM_CONSTANT_H
#define OSADAPTOR_IO_IPC_SHM__SHM_CONSTANT_H

#include <string>
#include "IO/ipc/IpcMediaInfo.h"

namespace osadaptor::ipc {

namespace shm {
const std::string kDefaultShmDirectory {"/tmp"};
const std::string kShmDomainStr{"osadaptor_shm_domain_"};
const std::string kShmPortStr{"_port_"};
const std::string kDefaultSharedMemoryPath {"/tmp/osadaptor_shm_domain_10_port_10"};

std::string kShmPathFormat {"%s/osadaptor_shm_domain_%u_port_%u"};
std::string kShmClientPathFormat {"%s/osadaptor_shm_domain_%u_port_%u_client_%u"};
std::string kShmPathFormatCpp20 {"{}/osadaptor_shm_domain_{}_port_{}"};  /** or fmt lib */
std::string kShmClientPathFormatCpp20 {"{}/osadaptor_shm_domain_{}_port_{}_client_{}"};  /** or fmt lib */

const std::string kReaderFormat {"%s/osadaptor_shm_domain_%u_port_%u_reader2writer.shm"};
const std::string kWriterFormat {"%s/osadaptor_shm_domain_%u_port_%u_writer2reader.shm"};

}  // namespace shm

} // namespace osadaptor::ipc

#endif // OSADAPTOR_IO_IPC_SHM__SHM_CONSTANT_H
