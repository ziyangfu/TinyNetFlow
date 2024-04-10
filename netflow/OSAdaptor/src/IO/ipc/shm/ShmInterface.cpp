//
// Created by fzy on 23-12-4.
//
#include "IO/ipc/shm/ShmInterface.h"
#include "IO/ipc/shm/PreDefineShm.h"

#include <spdlog/spdlog.h>
#include <unistd.h>

using namespace netflow::osadaptor::ipc::shm;

/*!
 * \brief 创建内存映射区
 * */
auto interface::mmap(int fd, std::size_t len) noexcept {
    void* const addrPtr { ::mmap(nullptr, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)};
    if (addrPtr == MAP_FAILED) {
        SPDLOG_ERROR("shared memory, mmap failed");
    }
    return reinterpret_cast<std::uint8_t*>(addrPtr);
}
/*!
 * \brief 释放内存映射区
 * */
auto interface::munmap(std::uint8_t *ptr, std::size_t len) noexcept {
    int ret = ::munmap(reinterpret_cast<void*>(ptr), len);
    if (ret == -1) {
        SPDLOG_ERROR("shared memory, munmap failed");
    }
    return ret;
}

/*!
 * \brief 设置文件大小
 * \details
 *       在创建文件后，调用该函数设置文件大小，如4096， 然后创建共享内存映射
 * */
void interface::ftruncate(int fd, std::size_t fileSize) noexcept {
    int ret = ::ftruncate(fd, static_cast<off_t>(fileSize));
    if (ret == -1) {
        SPDLOG_ERROR("shared memory, failed to set file size");
    }
}

/*!
 * \brief 一站式共享内存创建服务
 * \details
 *      1. 创建文件
 *      2. 修改权限
 *      3. 设置映射文件大小
 *      4. 内存映射
 *      5. 获取pid
 *      */
void interface::createSharedMemory() {
    int fd = createFile(kDefaultSharedMemoryPath.c_str());

    std::size_t const interfaceSize {}; /** 计算需要共享内存映射区的大小，传入的media_size+ring buffer大小 */
    mode_t mode {0666};
    chmod(kDefaultSharedMemoryPath.c_str(), mode);
    ftruncate(fd, interfaceSize);
    mmap(fd, interfaceSize);
    const std::uint32_t pid { static_cast<std::uint32_t>(::getpid())};
}


/** ---------------------------- OS 接口 -------------------------------------------- */
/*!
 * \brief 一站式共享内存创建服务（已有fd）*/

/*!
 * \brief 创建共享内存所需的文件
 * */
int interface::createFile(const char* filePath) noexcept {
    int fd = ::open(filePath, O_RDWR | O_CREAT, 0666);
    if (fd == -1) {
        SPDLOG_ERROR("shared memory, failed to create file");
    }
    return fd;
}

/*!
 * \brief 获取文件大小
 * */
auto interface::getFileSize(int fd) noexcept {
    struct stat info {};
    int ret = ::fstat(fd, &info);
    if (ret == -1) {
        SPDLOG_ERROR("failed to get file size");
    }
    return info.st_size;
}

/*!
 * \brief 获取文件模式
 * */
auto interface::getFileMode(const char *filePath) noexcept {
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
auto interface::access(const char *filePath) noexcept -> void {
   int ret = ::access(filePath, F_OK);
   if (ret == -1) {
       SPDLOG_ERROR("failed to access");
   }
}

/*!
 * \brief 设置文件权限与模式
 * */
void interface::chmod(const char *filePath, mode_t mode) noexcept {
    int ret = ::chmod(filePath, mode);
    if (ret == -1) {
        SPDLOG_ERROR("failed to set file access permission");
    }
}

/*!
 * \brief 修改文件名字
 * */
void interface::rename(const char *oldFileName, const char *newFileName) noexcept {
    int ret = ::rename(oldFileName, newFileName);
    if (ret == -1) {
        SPDLOG_ERROR("failed to rename the file");
    }

    memfd_create("shma", F_SEAL_SHRINK | F_SEAL_GROW);
}



