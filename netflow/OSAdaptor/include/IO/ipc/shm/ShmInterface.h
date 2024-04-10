//
// Created by fzy on 23-12-4.
//

#ifndef TINYNETFLOW_OSADAPTOR_SHMINTERFACE_H
#define TINYNETFLOW_OSADAPTOR_SHMINTERFACE_H

#include <sys/mman.h>
#include <string>

#include "IO/ipc/shm/IpcTypeInfo.h"

namespace netflow::osadaptor::ipc::shm {

namespace interface {

auto mmap(int fd, std::size_t len) noexcept;
auto munmap(std::uint8_t* ptr, std::size_t len) noexcept;
void ftruncate(int fd, std::size_t fileSize) noexcept;
void createSharedMemory();
/** ---------------------------- OS 接口 ----------------------------- */
auto access(const char* filePath) noexcept -> void;
int createFile(const char* filePath) noexcept;
auto getFileSize(int fd) noexcept;
auto getFileMode(const char* filePath) noexcept;
void chmod(const char* filePath, mode_t mode) noexcept;
void rename(const char* oldFileName, const char* newFileName) noexcept;

}  // namespace interface
}  // namespace netflow::osadaptor::ipc::shm


#endif //TINYNETFLOW_OSADAPTOR_SHMINTERFACE_H
