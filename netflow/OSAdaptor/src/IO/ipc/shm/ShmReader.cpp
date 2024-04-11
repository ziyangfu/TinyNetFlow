//
// Created by fzy on 23-12-4.
//

#include "IO/ipc/shm/ShmReader.h"


using namespace netflow::osadaptor::ipc;

auto ShmReader::mmap(int fd, std::size_t len) noexcept {

}


ShmReader::ShmReader(const std::string &sharedMemoryPath) : sharedMemoryPath_(sharedMemoryPath) {}

const std::string &ShmReader::getSharedMemoryPath() const {
    return sharedMemoryPath_;
}