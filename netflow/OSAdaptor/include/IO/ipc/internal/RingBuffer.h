//
// Created by fzy on 2024/12/18.
//

/*!
 * 实现一个固定大小的环形缓冲区，共享内存映射到环形缓冲区
 * */

#ifndef OSADAPTOR_IO_IPC_INTERNAL__RING_BUFFER_H
#define OSADAPTOR_IO_IPC_INTERNAL__RING_BUFFER_H

#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <stdexcept>
#include <iostream>

namespace osadaptor::ipc {
namespace internal {

class RingBuffer final {
public:
    // 构造函数，初始化环形缓冲区
    RingBuffer(const char* shmName, size_t capacity);

    // 析构函数，清理资源
    ~RingBuffer();

    // 写入数据到环形缓冲区
    bool write(const void* data, size_t dataSize);

    // 从环形缓冲区读取数据
    bool read(void* data, size_t dataSize);

    // 检查环形缓冲区是否为空
    bool isEmpty() const;

    // 检查环形缓冲区是否已满
    bool isFull() const;

private:
    struct CircularBuffer {
        size_t capacity;
        size_t head;
        size_t tail;
        char buffer[1]; // 实际大小由构造函数决定
    };

    CircularBuffer* m_cb;
    int m_fd;
    size_t m_capacity;
};

}  // namespace internal
}  // namespace osadaptor::ipc




#endif //OSADAPTOR_IO_IPC_INTERNAL__RING_BUFFER_H


