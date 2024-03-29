//
// Created by fzy on 23-12-4.
//

#ifndef TINYNETFLOW_SHMREADER_H
#define TINYNETFLOW_SHMREADER_H

#include <sys/mman.h>
#include <string>

#include "preDefineShm.h"

namespace netflow::ipc {
/*!
 * \brief 共享内存数据读取端
 * */
class ShmReader {
public:
    ShmReader();
    ~ShmReader();

    int open();
    void start();
    void stop();
    void close();

    auto mmap(int fd, std::size_t len) noexcept;
private:
    const std::string sharedMemoryPath_;
};

}  // namespace netflow::ipc



#endif //TINYNETFLOW_SHMREADER_H
