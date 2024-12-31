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
 *      Linux 系统共享内存相关接口定义， 采用mmap共享内存区域，同步机制采用 POSIX 信号量
 * \todo
 *      1. 同步机制可以考虑 uds/tcp
 *      2. 考虑使用memfd，而不是mmap，字节在shmipc上碰到了fork导致的内存踩踏问题，建议使用memfd
 * \file
 *      ShmSysInterface.h
 * ----------------------------------------------------------------------------------------- */

#ifndef OSADAPTOR_IO_IPC_SHM_SHM_SYS_INTERFACE_H
#define OSADAPTOR_IO_IPC_SHM_SHM_SYS_INTERFACE_H

#include <sys/mman.h>
#include <semaphore.h>
#include <cstdarg>
#include <string>
#include <string_view>

#include <vector>

#include "IO/ipc/shm/ShmIdentifierInfo.h"

namespace osadaptor::ipc {

namespace shm {

void ftruncate(int fd, std::size_t fileSize) noexcept;
auto access(const char* filePath) noexcept -> void;
int createFile(const char* filePath) noexcept;
int openFile(const char* filePath) noexcept;
int closeFile(int fd) noexcept;

auto getFileSize(int fd) noexcept;

auto getFileMode(const char* filePath) noexcept;
void setFileMode(const char* filePath, mode_t mode) noexcept;
void rename(const char* oldFileName, const char* newFileName) noexcept;

int createSharedMemory(ShmIdentifierInfo& shmInfo);
int openSharedMemory(ShmIdentifierInfo& shmInfo);
void closeSharedMemory(std::uint8_t* addr, ShmIdentifierInfo& shmInfo);
void closeSharedMemoryAll(int fd, std::uint8_t* addr, ShmIdentifierInfo& shmInfo);

std::uint8_t* mapSharedMemory(int fd, size_t size);
int unmapSharedMemory(std::uint8_t *addr, size_t size);
void unlinkSharedMemory(const char *filePath);
/** 信号量部分，使用 pthread mutex也是可以实现进程互斥锁的。 pthread_mutexattr_setpshared：PTHREAD_PROCESS_SHARED */
sem_t* openSemaphore(const char* name, int initialValue);
sem_t* openBinarySemaphore(const char* name);
void destroySemaphore(const char* name, sem_t *sem);
void waitSemaphore(sem_t* sem); /** V -1 */
void tryWaitSemaphore(sem_t* sem);  /** 超时 */
void postSemaphore(sem_t* sem); /** P +1 */
int getSemValue(sem_t* sem, int* sval);

template <typename... Args>
std::string formatStringCpp20(std::string_view format, Args&&... args);
std::string formatString(const std::string& format, ...);

void createShmCfgFile(const std::string& path);
void deleteShmCfgFile(const std::string& path);
void readShmCfgFile(const std::string& path, std::vector<ShmIdentifierInfo>& shmInfoList);
void writeShmCfgFile(const std::string& path, const std::vector<ShmIdentifierInfo>& shmInfoList);

}  // namespace shm
}  // namespace osadaptor::ipc


#endif //OSADAPTOR_IO_IPC_SHM_SHM_SYS_INTERFACE_H
