//
// Created by fzy on 23-12-4.
//

#ifndef TINYNETFLOW_OSADAPTOR_SHMREADER_H
#define TINYNETFLOW_OSADAPTOR_SHMREADER_H

#include <sys/mman.h>
#include <string>

#include "IO/ipc/shm/PreDefineShm.h"

namespace netflow::osadaptor::ipc {
/*!
 * \brief 共享内存数据读取端
 * */
class ShmReader {
public:
    ShmReader();
    ~ShmReader();

    explicit ShmReader(const std::string &sharedMemoryPath);

    const std::string &getSharedMemoryPath() const;

    int open();
    void start();
    void stop();
    void close();

    auto mmap(int fd, std::size_t len) noexcept;
private:
    const std::string sharedMemoryPath_;
};

}  // namespace netflow::osadaptor::ipc



#endif //TINYNETFLOW_OSADAPTOR_SHMREADER_H
