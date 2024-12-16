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
 *      共享内存读端
 * \file
 *      ShmReader.h
 * ----------------------------------------------------------------------------------------- */


#ifndef OSADAPTOR_IO_IPC_SHM__SHM_READER_H
#define OSADAPTOR_IO_IPC_SHM__SHM_READER_H

#include <string>
#include "IO/ipc/shm/ShmSysInterface.h"
#include "IO/ipc/shm/ShmConstant.h"

/**
 * 构造函数和析构函数：
ShmReader()：默认构造函数，初始化成员变量。
ShmReader(const std::string &sharedMemoryPath)：带参数的构造函数，初始化成员变量并设置共享内存路径。
~ShmReader()：析构函数，确保在对象销毁前关闭共享内存。
成员函数：
getSharedMemoryPath()：返回共享内存路径。
open()：打开共享内存文件并映射到内存。
start()：启动读取操作。
stop()：停止读取操作。
close()：关闭共享内存并清理资源。
readData(void* buffer, size_t bufferSize)：从共享内存读取数据到缓冲区，并使用信号量进行同步。
私有成员函数：
initSemaphore()：初始化信号量。
destroySemaphore()：销毁信号量。
成员变量：
sharedMemoryPath_：共享内存文件路径。
fd_：文件描述符。
mappedAddr_：映射的内存地址。
mappedSize_：映射的内存大小。
sem_：信号量指针。
isOpen_：指示共享内存是否已打开。
isRunning_：指示读取操作是否正在运行。
 */
namespace osadaptor::ipc {

class ShmReader {
public:
    ShmReader();
    ~ShmReader();
    explicit ShmReader(const std::string &sharedMemoryPath);
    const std::string &getSharedMemoryPath() const;


    bool connect();
    void disconnect();

    int open();
    void start();
    void stop();
    void close();
    void readData(void* buffer, size_t bufferSize);
private:
    void initSemaphore();
    void destroySemaphore();
private:
    const std::string sharedMemoryPath_;
    int fd_;
    void* mappedAddr_;
    size_t mappedSize_;
    sem_t* sem_;
    bool isOpen_;
    bool isRunning_;
};

}  // namespace netflow::osadaptor::ipc



#endif //OSADAPTOR_IO_IPC_SHM__SHM_READER_H
