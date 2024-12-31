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
 *      实现一个固定大小的环形缓冲区，共享内存映射到环形缓冲区
 * \file
 *      RingBuffer.h
 * ----------------------------------------------------------------------------------------- */

#ifndef OSADAPTOR_IO_IPC_INTERNAL__RING_BUFFER_H
#define OSADAPTOR_IO_IPC_INTERNAL__RING_BUFFER_H

#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <stdexcept>
#include <vector>


// 注意，越过初始点的时候， tail比head大
namespace osadaptor::ipc {
namespace internal {

class RingBuffer final {
public:
    // 构造函数，初始化环形缓冲区
    RingBuffer(const char* shmName, size_t capacity);

    // 析构函数，清理资源
    ~RingBuffer();

    void init();

    // 写入数据到环形缓冲区
    bool write(const void* data, size_t dataSize);

    // 从环形缓冲区读取数据
    bool read(void* data, size_t dataSize);

    // 检查环形缓冲区是否为空
    bool isEmpty() const;

    // 检查环形缓冲区是否已满
    bool isFull() const;

    size_t getFreeSpace();

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


    std::vector<std::uint8_t> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;
    size_t capacity_;
};

}  // namespace internal
}  // namespace osadaptor::ipc




#endif //OSADAPTOR_IO_IPC_INTERNAL__RING_BUFFER_H


