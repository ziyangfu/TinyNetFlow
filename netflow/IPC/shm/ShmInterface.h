//
// Created by fzy on 23-12-4.
//

#ifndef TINYNETFLOW_SHMINTERFACE_H
#define TINYNETFLOW_SHMINTERFACE_H

#include <sys/mman.h>
#include <string>

namespace netflow::ipc::shm {

auto mmap(int fd, std::size_t len) noexcept;
auto munmap(std::uint8_t* ptr, std::size_t len) noexcept;
auto getFileSize(int fd) noexcept;
int createFile(const char* filePath) noexcept;
void ftruncate(int fd, std::size_t fileSize) noexcept;

void createSharedMemory();

} // namespace netflow::ipc::shm

#endif //TINYNETFLOW_SHMINTERFACE_H
