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
 *      共享内存写端
 * \file
 *      ShmWriter.h
 * ----------------------------------------------------------------------------------------- */


#ifndef OSADAPTOR_IO_IPC_SHM__SHM_WRITER_H
#define OSADAPTOR_IO_IPC_SHM__SHM_WRITER_H

#include <string>
#include "IO/ipc/shm/ShmSysInterface.h"

/**
 * ShmWriter 类：
构造函数和析构函数：
ShmWriter()：默认构造函数，初始化成员变量。
ShmWriter(const std::string &sharedMemoryPath)：带参数的构造函数，初始化成员变量并设置共享内存路径。
~ShmWriter()：析构函数，确保在对象销毁前关闭共享内存。
成员函数：
getSharedMemoryPath()：返回共享内存路径。
open()：创建共享内存文件并映射到内存。
start()：启动写入操作。
stop()：停止写入操作。
close()：关闭共享内存并清理资源。
writeData(const void* buffer, size_t bufferSize)：将数据从缓冲区写入共享内存，并使用信号量进行同步。
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
isRunning_：指示写入操作是否正在运行。
使用例程：
创建 ShmWriter 和 ShmReader 对象，并指定相同的共享内存路径。
启动两个线程，一个用于写入数据，另一个用于读取数据。
在写入线程中，打开共享内存，启动写入操作，写入数据后停止并关闭共享内存。
在读取线程中，打开共享内存，启动读取操作，读取数据后停止并关闭共享内存。
主线程等待两个子线程完成。
 * */


namespace osadaptor::ipc {

class ShmWriter {
public:
    ShmWriter();
    explicit ShmWriter(const std::string &sharedMemoryPath);
    ~ShmWriter();

    const std::string &getSharedMemoryPath() const;
    int open();
    void start();
    void stop();
    void close();
    void writeMessage(const void* buffer, size_t bufferSize);
private:
    const std::string sharedMemoryPath_;
    int fd_;
    void* mappedAddr_;
    size_t mappedSize_;
    sem_t* sem_;

    bool isOpen_;
    bool isRunning_;

    void initSemaphore();
    void destroySemaphore();
};

}  // namespace osadaptor::ipc


#endif //OSADAPTOR_IO_IPC_SHM__SHM_WRITER_H
