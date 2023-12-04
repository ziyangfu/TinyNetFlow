//
// Created by fzy on 23-12-4.
//

#include "netflow/IPC/shm/ShmInterface.h"
#include "netflow/base/Logging.h"

#include <unistd.h>

using namespace netflow::ipc;
using namespace netflow::base;
/*!
 * \brief 创建内存映射区
 * */
auto shm::mmap(int fd, std::size_t len) noexcept {
    void* const addr { ::mmap(nullptr, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)};
    if (addr == MAP_FAILED) {
        STREAM_ERROR << "shared memory, mmap failed";
    }
    return addr;
}
/*!
 * \brief 释放内存映射区
 * */
auto shm::munmap(std::uint8_t *ptr, std::size_t len) noexcept {
    int ret = ::munmap(reinterpret_cast<void*>(ptr), len);
    if (ret == -1) {
        STREAM_ERROR << "shared memory, munmap failed";
    }
    return ret;
}

/*!
 * \brief 获取文件大小
 * */
auto shm::getFileSize(int fd) noexcept {
    struct stat info {};
    int ret = ::fstat(fd, &info);
    if (ret == -1) {
        STREAM_ERROR << "failed to get file size";
    }
    return info.st_size;
}

/*!
 * \brief 创建共享内存所需的文件
 * */
int shm::createFile(const char* filePath) noexcept {
    int fd = ::open(filePath, O_RDWR | O_CREAT, 0666);
    if (fd == -1) {
        STREAM_ERROR << "shared memory, failed to create file";
    }
    return fd;
}

/*!
 * \brief 设置文件大小
 * */
void shm::ftruncate(int fd, std::size_t fileSize) noexcept {
    int ret = ::ftruncate(fd, static_cast<off_t>(fileSize));
    if (ret == -1) {
        STREAM_ERROR << "shared memory, failed to set file size";
    }
}

/*!
 * \brief 一站式共享内存创建服务
 * \details
 *      1. 创建文件
 *      2. 修改权限
 *      3. 设置映射文件大小
 *      4. 内存映射
 *      */
void shm::createSharedMemory() {

}
