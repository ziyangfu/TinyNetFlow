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

/**
使用C++17实现一个RingBuufer，包含2个文件，RingBuffer.h与RingBuffer.cpp。
 在RingBuffer.h中设计一个RingBuffer类，采用std::array作为底层存储容器，大小通过构造函数传入，默认大小为64KB，
 std::array前面8个字节为reserve区域，里面包含2个 std::atmoic<std::uint32_t>的index，分为head和tail。
 RingBuffer需要实现常见的读写操作以及head与tail的控制操作，包括write、read、isEmpty、isFull、usedCapacity、freeCapacity、bufferCapacity等。
 RingBuffer最终会用在mmap共享内存映射中，mmap返回的addr，即std::array的首地址。
 若std::array不合适，可以替换为其他合适的容器

*/

#ifndef OSADAPTOR_IO_IPC_INTERNAL__RING_BUFFER_H
#define OSADAPTOR_IO_IPC_INTERNAL__RING_BUFFER_H

#include <atomic>
#include <vector>
#include <cstdint>
#include <stdexcept>

#include <span>

namespace osadaptor::ipc {
namespace internal {
class RingBuffer {
public:
    RingBuffer(const char* shm_name, size_t buffer_size);
    ~RingBuffer();

    bool write(const void* data, size_t len);
    bool read(void* data, size_t len);

private:
    std::span<char> buffer__;
    char* buffer_;
    std::atomic<size_t>* head_;
    std::atomic<size_t>* tail_;
    size_t buffer_size_;
    int shm_fd_;

    size_t get_free_space() const;
    size_t get_used_space() const;
};


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




https://github.com/MengRao/SPSC_Queue/blob/master/SPSCQueue.h
SPSC_Queue
/*
MIT License

Copyright (c) 2018 Meng Rao <raomeng1@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once
#include <atomic>

template<class T, uint32_t CNT>
class SPSCQueue
{
public:
    static_assert(CNT && !(CNT & (CNT - 1)), "CNT must be a power of 2");

    T* alloc() {
        if (write_idx - read_idx_cach == CNT) {
            read_idx_cach = ((std::atomic<uint32_t>*)&read_idx)->load(std::memory_order_consume);
            if (__builtin_expect(write_idx - read_idx_cach == CNT, 0)) { // no enough space
                return nullptr;
            }
        }
        return &data[write_idx % CNT];
    }

    void push() {
        ((std::atomic<uint32_t>*)&write_idx)->store(write_idx + 1, std::memory_order_release);
    }

    template<typename Writer>
    bool tryPush(Writer writer) {
        T* p = alloc();
        if (!p) return false;
        writer(p);
        push();
        return true;
    }

    template<typename Writer>
    void blockPush(Writer writer) {
        while (!tryPush(writer))
            ;
    }

    T* front() {
        if (read_idx == ((std::atomic<uint32_t>*)&write_idx)->load(std::memory_order_acquire)) {
            return nullptr;
        }
        return &data[read_idx % CNT];
    }

    void pop() {
        ((std::atomic<uint32_t>*)&read_idx)->store(read_idx + 1, std::memory_order_release);
    }

    template<typename Reader>
    bool tryPop(Reader reader) {
        T* v = front();
        if (!v) return false;
        reader(v);
        pop();
        return true;
    }

private:
    alignas(128) T data[CNT] = {};

    alignas(128) uint32_t write_idx = 0;
    uint32_t read_idx_cach = 0; // used only by writing thread

    alignas(128) uint32_t read_idx = 0;
};




