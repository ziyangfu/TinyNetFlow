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
 *      Linux 系统共享内存相关接口定义，主要是mmap与信号量部分
 * \file
 *      ShmSysInterface.cpp
 * ----------------------------------------------------------------------------------------- */
#include "IO/ipc/shm/ShmSysInterface.h"
#include "IO/ipc/shm/ShmConstant.h"

#include <unistd.h>
// Check if C++20 or later is supported
#if __cplusplus >= 202002L
#include <format>
#endif

#if __cplusplus < 202002L
#ifdef UseFmtLibrary     /** TODO: import fmt lib */
#include <fmt/fmt.h>
#endif
#else
//const bool traditional = true;
#endif


#include <spdlog/spdlog.h>

using namespace osadaptor::ipc;

/*!
 * \brief 一站式共享内存创建服务, for writer
 * \details
 *      0. 组装文件绝对路径
 *      1. 创建文件
 *      2. 修改权限
 *      3. 设置映射文件大小
 *      4. 内存映射
 *      5. 获取pid
 *      6. 返回文件描述符
 *      */
int shm::createSharedMemory(ShmIdentifierInfo& shmInfo) {
    std::string filePath;
    if (shmInfo.path_.domain == kIpcIndexDomainPortMin && shmInfo.path_.port == kIpcIndexDomainPortMin) {
        filePath = kDefaultSharedMemoryPath;
    }
    else {
        filePath = formatString(kShmPathFormat, kDefaultShmDirectory.c_str(),
                                shmInfo.path_.domain, shmInfo.path_.port);
        //filePath = formatStringCpp20(kShmPathFormatCpp20, kDefaultShmDirectory,
        //                      shmInfo.path_.domain, shmInfo.path_.port);
    }

    int fd = createFile(filePath.c_str());
    if (fd == -1) {
        return -1;
    }

    mode_t mode {0666};
    setFileMode(filePath.c_str(), mode);

    ftruncate(fd, shmInfo.size_);
    std::uint8_t* shmAddr = mapSharedMemory(fd, shmInfo.size_);
    if (!shmAddr){
        closeSharedMemoryAll(fd, shmAddr, shmInfo);
        return -1;
    }
    const std::uint32_t pid { static_cast<std::uint32_t>(::getpid())};
    shmInfo.pid_ = pid;
    return fd;
}
/*!
 * \brief 使用memfd创建共享内存. memfd_create(path) + mmap(addr, size), for reader
 * \todo
 * */
int shm::createMemFd() {
    int fd = ::memfd_create("tiny_netflow_memfd", 0);
    return fd;
}


/*!
 * \brief 打开共享内存. open(path) + mmap(addr, size), for reader
 * */
int shm::openSharedMemory(osadaptor::ipc::shm::ShmIdentifierInfo &shmInfo) {
    std::string filePath;
    if (shmInfo.path_.domain == kIpcIndexDomainPortMin && shmInfo.path_.port == kIpcIndexDomainPortMin) {
        filePath = kDefaultSharedMemoryPath;
    }
    else {
        filePath = formatString(kShmPathFormat, kDefaultShmDirectory.c_str(),
                                shmInfo.path_.domain, shmInfo.path_.port);
        //filePath = formatStringCpp20(kShmPathFormatCpp20, kDefaultShmDirectory,
        //                      shmInfo.path_.domain, shmInfo.path_.port);
    }
    int fd = openFile(filePath.c_str());
    std::uint8_t* shmAddr = mapSharedMemory(fd, shmInfo.size_);
    if (!shmAddr){
        closeSharedMemoryAll(fd, shmAddr, shmInfo);
        return -1;
    }
    return fd;
}

/*!
 * \brief 关闭共享内存. unmap(addr, size) + unlink(path)
 * \details
 * 1. 获取文件绝对路径
 * 2. 释放内存映射
 * 3. 删除文件（文件描述符由上层保存与关闭）
 * */
void shm::closeSharedMemory(std::uint8_t* addr, ShmIdentifierInfo& shmInfo) {
    std::string filePath;
    if (shmInfo.path_.domain == kIpcIndexDomainPortMin && shmInfo.path_.port == kIpcIndexDomainPortMin) {
        filePath = kDefaultSharedMemoryPath;
    }
    else {
        filePath = formatString(kShmPathFormat, kDefaultShmDirectory.c_str(),
                                shmInfo.path_.domain, shmInfo.path_.port);
        //filePath = formatStringCpp20(kShmPathFormatCpp20, kDefaultShmDirectory,
        //                      shmInfo.path_.domain, shmInfo.path_.port);
    }

    unmapSharedMemory(addr, shmInfo.size_);
    unlinkSharedMemory(filePath.c_str());
}
/*!
 * \brief 一站式关闭共享内存， unmap(addr, size) + unlink(path) + close(fd)
 * */
void shm::closeSharedMemoryAll(int fd, std::uint8_t *addr, osadaptor::ipc::shm::ShmIdentifierInfo &shmInfo) {
    closeSharedMemory(addr, shmInfo);
    closeFile(fd);
}

/*!
 * \brief 创建内存映射区
 * \return 返回映射的地址
 * \details
 *      将共享内存文件映射到进程的地址空间。
 *      文件描述符映射，注意mmap返回的是void*指针，后面要转换为uint8_t*
 *      mmap nullptr: 让操作系统自动选择合适的内存地址
 *           0: offset：表示映射文件的偏移量，一般设置为 0 表示从文件头部开始映射
 * */
std::uint8_t* shm::mapSharedMemory(int fd, size_t size) {
    void* addrPtr {nullptr};
    addrPtr = ::mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addrPtr == MAP_FAILED) {
        SPDLOG_ERROR("shared memory, mmap failed");
    }
    return reinterpret_cast<std::uint8_t*>(addrPtr);
}
/*!
 * \brief 解除共享内存的映射，释放内存映射区
 * \details munmap 接收的是 void* 指针，所以要将 uint8_t 指针转换为 void*
 * */
int shm::unmapSharedMemory(std::uint8_t *addr, size_t size) {
    int ret = ::munmap(reinterpret_cast<void*>(addr), size);
    if (ret == -1) {
        SPDLOG_ERROR("shared memory, munmap failed");
    }
    return ret;

}

/*!
 * \brief 删除共享内存文件
 * */
void shm::unlinkSharedMemory(const char *filePath) {
    if (unlink(filePath) == -1) {
        SPDLOG_ERROR("Failed to delete shared memory file: {}", filePath);
    }
}

/*!
 * \brief 创建或打开一个命名的信号量， writer使用
 * \arg @name: 信号量的名称
 * \arg @initialValue: 信号量初始值
 * \details oflag：标志位，可以是
 *                 O_CREAT（创建信号量）
 *                 O_EXCL（与O_CREAT一起使用，如果信号量已经存在则失败）
 *                 O_RDWR（读写模式）等
 * \return 成功返回指向信号量对象的指针
 *         失败返回 nullptr
 * */
sem_t *shm::openSemaphore(const char *name, int initialValue) {
    sem_t* sem = sem_open(name, O_CREAT | O_EXCL, 0666, initialValue);
    if (sem == SEM_FAILED) {
        SPDLOG_ERROR("Failed to initialize semaphore: {}", name);
        return nullptr;
    }
    return sem;
}

sem_t *shm::createSemaphore(const char *name, int initialValue) {
    sem_t sem;
    sem_init(&sem, 1, initialValue);
}
/*!
 * \brief 二元信号量，即进程级别的互斥量
 * */
sem_t *shm::openBinarySemaphore(const char *name) {
    return openSemaphore(name, 1);
}
/*!
 * \brief 销毁信号量(关闭并删除一个命名的信号量)
 * \return void
 * */
void shm::destroySemaphore(const char* name, sem_t *sem) {
    if (sem_close(sem) == -1) {
        SPDLOG_ERROR("Failed to close semaphore");
    }
    if (sem_unlink(name) == -1) {
        SPDLOG_ERROR("Failed to unlink semaphore: {}", name);
    }

}
/*!
 * \brief
 *          等待信号量，如果信号量的值大于0，则将其减1；
 *          否则调用进程阻塞直到信号量的值大于0
 *          对于二元信号量，该函数等价于P操作
 * */
void shm::waitSemaphore(sem_t *sem) {
    if (sem_wait(sem) == -1) {
        SPDLOG_ERROR("Failed to wait on semaphore");
    }
}

/*!
 * \brief  尝试等待信号量，如果信号量的值大于0，则将其减1，并返回0
 **/
void shm::tryWaitSemaphore(sem_t *sem) {

}

/*!
 * \brief V操作，将信号量的值加1，并唤醒等待该信号量的进程
 * */
void shm::postSemaphore(sem_t *sem) {
    if (sem_post(sem) == -1) {
        SPDLOG_ERROR("Failed to post semaphore");
    }
}

/*!
 * \brief 获取信号量的值
 * */
int shm::getSemValue(sem_t *sem, int *sval) {
    if (sem_getvalue(sem, sval) == -1) {
        SPDLOG_ERROR("Failed to get semaphore value, errno: {}", errno);
        return errno;
    }
    else {
        return 0;
    }
}
/*!
 * \brief 根据format格式，填入参数，返回完整的字符串， C++20 format库
 * */
template<typename ...Args>
std::string shm::formatStringCpp20(std::string_view format, Args &&...args) {
#if __cplusplus >= 202002L
    return std::vformat(format, std::make_format_args(args...));
#else
    /** TODO: return fmt::fmt(...) */
#endif
}

/*!
 * \brief 根据format格式，填入参数，返回完整的字符串
 * \details
 *      - 低于C++20，且没有fmt库 则使用vsnprintf 函数
 *      - va_list 是 C 语言中用来处理可变参数列表的一个特殊类型。当你需要编写一个函数，
 *          且该函数接受数量不确定的参数时，va_list 就非常有用
 * */
std::string shm::formatString(const std::string &format, ...) {
    std::vector<char> buffer(100);
    std::string result;
    va_list args;
    va_start(args, format.c_str());   /** format之后即可变参数列表 */
    int ret = vsnprintf(buffer.data(), buffer.size(), format.c_str(), args);
    if (ret < 0) {
        throw std::runtime_error("vsnprintf failed");
    }
    va_end(args);
    if (static_cast<size_t>(ret) < buffer.size()) {
        result = std::string(buffer.data(), ret);
    }
    return result;
}

/*!
 * \brief /tmp/osadaptor_domain_xx_port_xx.cfg
 *          哪种IPC方式，uds or shm
 *          size多大
 * */
void shm::createShmCfgFile(const std::string &path) {

}

void shm::readShmCfgFile(const std::string &path,
                         std::vector<ShmIdentifierInfo> &shmInfoList) {

}

void shm::writeShmCfgFile(const std::string &path,
                          const std::vector<ShmIdentifierInfo> &shmInfoList) {

}

void shm::deleteShmCfgFile(const std::string &path) {

}

/*!
 * \brief 设置文件大小
 * \details
 *       在创建文件后，调用该函数设置文件大小，如4096， 然后创建共享内存映射
 * */
void shm::ftruncate(int fd, std::size_t fileSize) noexcept {
    int ret = ::ftruncate(fd, static_cast<off_t>(fileSize));
    if (ret == -1) {
        SPDLOG_ERROR("shared memory, failed to set file size");
    }
}

/*!
 * \brief 文件不存在时，创建共享内存所需的文件
 * */
int shm::createFile(const char* filePath) noexcept {
    int fd = ::open(filePath, O_RDWR | O_CREAT, 0666);
    if (fd == -1) {
        SPDLOG_ERROR("shared memory, failed to create file");
    }
    return fd;
}

/*!
 * \brief 文件存在时，打开文件
 * */
int shm::openFile(const char *filePath) noexcept {
    int fd = ::open(filePath, O_RDWR, 0666);
    if (fd == -1) {
        SPDLOG_ERROR("shared memory, failed to open file");
    }
    return fd;
}

int shm::closeFile(int fd) noexcept {
    return ::close(fd);
}

/*!
 * \brief 获取文件大小
 * */
auto shm::getFileSize(int fd) noexcept {
    struct stat info {};
    int ret = ::fstat(fd, &info);
    if (ret == -1) {
        SPDLOG_ERROR("failed to get file size");
    }
    return info.st_size;
}

/*!
 * \brief 获取文件模式
 * \details
    文件类型的位标志通常位于 mode_t 的高几位。以下是常见的文件类型及其对应的位标志：
    S_IFSOCK - 套接字 (Socket)
    S_IFLNK - 符号链接 (Symbolic link)
    S_IFREG - 普通文件 (Regular file)
    S_IFBLK - 块设备 (Block device)
    S_IFDIR - 目录 (Directory)
    S_IFCHR - 字符设备 (Character device)
    S_IFIFO - 命名管道 (Named pipe)
    权限的位标志通常位于 mode_t 的低三位。以下是常见的权限及其对应的位标志：
    权限：读、写、执行
 * */
auto shm::getFileMode(const char *filePath) noexcept {
    struct stat info {};
    int ret = ::stat(filePath, &info);
    if (ret == -1) {
        SPDLOG_ERROR("failed to get file size");
    }
    return info.st_mode;
}

/*!
 * \brief 检查调用进程是否可以对指定的文件执行某种操作,检查文件是否存在
 * \details
 *      R_OK:读许可， W_OK：写许可， X_OK：执行许可， F_OK：文件是否存在
 *      测试文件是否存在，见 https://blog.csdn.net/tigerjibo/article/details/11712039
 *       */
auto shm::access(const char *filePath) noexcept -> void {
    int ret = ::access(filePath, F_OK);
    if (ret == -1) {
        SPDLOG_ERROR("failed to access");
    }
}

/*!
 * \brief 设置文件权限与模式
 * */
void shm::setFileMode(const char *filePath, mode_t mode) noexcept {
    int ret = ::chmod(filePath, mode);
    if (ret == -1) {
        SPDLOG_ERROR("failed to set file access permission");
    }
}

/*!
 * \brief 修改文件名字
 * \details
         memfd_create 函数用于在内存中创建一个匿名文件，并返回一个文件描述符。
        "shma" 是文件的名称，这个名称主要用于调试目的。
         F_SEAL_SHRINK 和 F_SEAL_GROW 是密封标志，分别表示不允许缩小和扩大文件的大小
 * */
void shm::rename(const char *oldFileName, const char *newFileName) noexcept {
    int ret = ::rename(oldFileName, newFileName);
    if (ret == -1) {
        SPDLOG_ERROR("failed to rename the file");
    }

    memfd_create("shma", F_SEAL_SHRINK | F_SEAL_GROW);
}




