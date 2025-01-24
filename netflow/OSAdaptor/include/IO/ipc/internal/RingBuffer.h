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
 *      实现一个用于映射共享内存的环形缓冲区
 * \file
 *      RingBuffer.h
 * ----------------------------------------------------------------------------------------- */

#ifndef OSADAPTOR_IO_IPC_INTERNAL__RING_BUFFER_H
#define OSADAPTOR_IO_IPC_INTERNAL__RING_BUFFER_H

#include <atomic>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <span>

namespace osadaptor::ipc {
namespace internal {

class RingBuffer final {
public:
    // 构造函数，默认大小为 64KB
    explicit RingBuffer(const char* address);
    // 写入数据到 RingBuffer
    bool write(const void* data, std::size_t size);
    // 从 RingBuffer 读取数据
    bool read(void* data, std::size_t size);
    // 判断 RingBuffer 是否为空
    bool isEmpty() const;
    // 判断 RingBuffer 是否已满
    bool isFull() const;
    // 获取已使用的容量
    std::size_t usedCapacity() const;
    // 获取剩余的可用容量
    std::size_t freeCapacity() const;
    // 获取 RingBuffer 的总容量
    std::size_t bufferCapacity() const;
    // 获取底层数据的指针（用于 mmap 共享内存）
    char* data();

private:
    // 计算可用空间
    std::size_t getAvailableSpace() const;
private:
    char* buffer_;
    std::size_t size_;
    // 头部和尾部索引（原子操作）
    std::atomic<std::uint32_t> head_;
    std::atomic<std::uint32_t> tail_;
};

}  // namespace internal
}  // namespace osadaptor::ipc

#endif //OSADAPTOR_IO_IPC_INTERNAL__RING_BUFFER_H