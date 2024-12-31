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
 *      共享内存终端节点，支持读取和写入共享内存，双向通信
 * \file
 *      ShmEndpoint.h
 * ----------------------------------------------------------------------------------------- */
 
 /*!
 https://github.com/mutouyun/cpp-ipc/blob/master/demo/chat/main.cpp
 https://github.com/yurivarvak/shmipc/blob/master/testclient.c
 https://github.com/cfallin/shmipclib/blob/master/shm.h
 https://blog.csdn.net/yueguangmuyu/article/details/118256035 [mmap()共享内存详解]
 */


#ifndef OSADAPTOR_IO_IPC_SHM__SHM_ENDPOINT_H
#define OSADAPTOR_IO_IPC_SHM__SHM_ENDPOINT_H

#include "IO/ipc/shm/ShmReader.h"
#include "IO/ipc/shm/ShmWriter.h"

namespace osadaptor {
namespace ipc {

class ShmEndpoint {
public:
    ShmEndpoint();
    ShmEndpoint(const std::string& shmPath, const std::string& shmName, std::uint32_t shmSize);
    ~ShmEndpoint();

    void start();
    void stop();
    void readMessage(void* buffer, size_t bufferSize);
    void writeMessage(const void* buffer, size_t bufferSize);
private:
    ShmReader reader_;
    ShmWriter writer_;
};

} // ipc
} // osadaptor

#endif //OSADAPTOR_IO_IPC_SHM__SHM_ENDPOINT_H
