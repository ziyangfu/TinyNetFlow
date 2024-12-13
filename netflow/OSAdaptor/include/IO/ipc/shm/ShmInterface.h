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
 *      Linux 系统共享内存相关接口定义， 采用mmap共享内存区域，采用信号量进行进程同步
 * \file
 *      ShmInterface.h
 * ----------------------------------------------------------------------------------------- */

#ifndef OSADAPTOR_IO_IPC_SHM_SHMINTERFACE_H
#define OSADAPTOR_IO_IPC_SHM_SHMINTERFACE_H

#include <sys/mman.h>
#include <semaphore.h>
#include <string>

#include "IO/ipc/shm/IpcTypeInfo.h"

namespace osadaptor::ipc {

namespace shm {


int createSharedMemory(const char* name, size_t size);
int openSharedMemory(const char* name);
void* mapSharedMemory(int fd, size_t size);
void unmapSharedMemory(void* addr, size_t size);
void closeSharedMemory(int fd);
void deleteSharedMemory(const char* name);

sem_t* initSemaphore(const char* name, int initialValue);
void destroySemaphore(sem_t* sem);
void waitSemaphore(sem_t* sem);
void postSemaphore(sem_t* sem); /** 释放信号量 */


/** ---------------------------- OS 接口 ----------------------------- */
auto mmap(int fd, std::size_t len) noexcept;
auto munmap(std::uint8_t* ptr, std::size_t len) noexcept;
void ftruncate(int fd, std::size_t fileSize) noexcept;

auto access(const char* filePath) noexcept -> void;
int createFile(const char* filePath) noexcept;
auto getFileSize(int fd) noexcept;
auto getFileMode(const char* filePath) noexcept;
void chmod(const char* filePath, mode_t mode) noexcept;
void rename(const char* oldFileName, const char* newFileName) noexcept;

}  // namespace shm
}  // namespace osadaptor::ipc


#endif //OSADAPTOR_IO_IPC_SHM_SHMINTERFACE_H
