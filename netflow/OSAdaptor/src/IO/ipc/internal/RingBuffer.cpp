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
 *      RingBuffer.cpp
 * ----------------------------------------------------------------------------------------- */
#include "IO/ipc/internal/RingBuffer.h"
#include <cstring>

namespace osadaptor::ipc {
namespace internal {

RingBuffer::RingBuffer(std::size_t capacity)
        : buffer_(capacity + 8), // 额外保留 8 字节用于 head 和 tail
          head_(0),
          tail_(0),
          capacity_(capacity)
{

}

/*!
 * \brief 写入数据
 * */
bool RingBuffer::write(const void* data, std::size_t size) {
    if (size > freeCapacity()) {
        return false; // 空间不足
    }

    std::uint32_t head = head_.load(std::memory_order_relaxed);
    std::uint32_t tail = tail_.load(std::memory_order_relaxed);

    // 计算可用空间
    std::size_t availableSpace = getAvailableSpace();

    if (size > availableSpace) {
        return false; // 空间不足
    }

    // 写入数据
    std::size_t firstPart = std::min(size, capacity_ - tail);
    std::memcpy(buffer_.data() + tail + 8, data, firstPart);
    if (size > firstPart) {
        std::memcpy(buffer_.data() + 8, static_cast<const char*>(data) + firstPart, size - firstPart);
    }

    // 更新 tail
    tail_.store((tail + size) % capacity_, std::memory_order_release);

    return true;
}

// 读取数据
bool RingBuffer::read(void* data, std::size_t size) {
    if (size > usedCapacity()) {
        return false; // 数据不足
    }

    std::uint32_t head = head_.load(std::memory_order_relaxed);
    std::uint32_t tail = tail_.load(std::memory_order_acquire);

    // 计算可用数据
    std::size_t availableData = (tail >= head) ? (tail - head) : (capacity_ - head + tail);

    if (size > availableData) {
        return false; // 数据不足
    }

    // 读取数据
    std::size_t firstPart = std::min(size, capacity_ - head);
    std::memcpy(data, buffer_.data() + head + 8, firstPart);
    if (size > firstPart) {
        std::memcpy(static_cast<char*>(data) + firstPart, buffer_.data() + 8, size - firstPart);
    }

    // 更新 head
    head_.store((head + size) % capacity_, std::memory_order_release);

    return true;
}

// 判断是否为空
bool RingBuffer::isEmpty() const {
    return head_.load(std::memory_order_acquire) == tail_.load(std::memory_order_acquire);
}

// 判断是否已满
bool RingBuffer::isFull() const {
    return usedCapacity() == capacity_;
}

// 获取已使用的容量
std::size_t RingBuffer::usedCapacity() const {
    std::uint32_t head = head_.load(std::memory_order_acquire);
    std::uint32_t tail = tail_.load(std::memory_order_acquire);

    if (tail >= head) {
        return tail - head;
    } else {
        return capacity_ - head + tail;
    }
}

// 获取剩余的可用容量
std::size_t RingBuffer::freeCapacity() const {
    return capacity_ - usedCapacity();
}

// 获取 RingBuffer 的总容量
std::size_t RingBuffer::bufferCapacity() const {
    return capacity_;
}

// 获取底层数据的指针
char* RingBuffer::data() {
    return buffer_.data();
}

// 计算可用空间
std::size_t RingBuffer::getAvailableSpace() const {
    std::uint32_t head = head_.load(std::memory_order_acquire);
    std::uint32_t tail = tail_.load(std::memory_order_acquire);

    if (tail >= head) {
        return capacity_ - (tail - head);
    } else {
        return head - tail;
    }
}



RingBuffer::RingBuffer(const char* shm_name, size_t buffer_size) : buffer_size_(buffer_size) {
    // Open the shared memory object
    shm_fd_ = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    if (shm_fd_ == -1) {
        throw std::runtime_error("Failed to open shared memory");
    }

    // Configure the size of the shared memory object
    if (ftruncate(shm_fd_, buffer_size_ + 8) == -1) {
        throw std::runtime_error("Failed to set size of shared memory");
    }

    // Map the shared memory object into the address space
    buffer_ = static_cast<char*>(mmap(nullptr, buffer_size_ + 8, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd_, 0));
    if (buffer_ == MAP_FAILED) {
        throw std::runtime_error("Failed to map shared memory");
    }

    // Initialize head and tail pointers
    head_ = reinterpret_cast<std::atomic<size_t>*>(buffer_);
    tail_ = reinterpret_cast<std::atomic<size_t>*>(buffer_ + sizeof(std::atomic<size_t>));
    *head_ = 0;
    *tail_ = 0;
}

RingBuffer::~RingBuffer() {
    munmap(buffer_, buffer_size_ + 8);
    close(shm_fd_);
}

bool RingBuffer::write(const void* data, size_t len) {
    size_t head = head_->load(std::memory_order_relaxed);
    size_t tail = tail_->load(std::memory_order_acquire);
    size_t free_space = get_free_space();

    if (len > free_space) {
        return false; // Not enough space
    }

    size_t first_part = std::min(len, buffer_size_ - tail);
    std::memcpy(buffer_ + 8 + tail, data, first_part);
    if (len > first_part) {
        std::memcpy(buffer_ + 8, static_cast<const char*>(data) + first_part, len - first_part);
    }

    tail_->store((tail + len) % buffer_size_, std::memory_order_release);
    return true;
}

bool RingBuffer::read(void* data, size_t len) {
    size_t head = head_->load(std::memory_order_acquire);
    size_t tail = tail_->load(std::memory_order_relaxed);
    size_t used_space = get_used_space();

    if (len > used_space) {
        return false; // Not enough data
    }

    size_t first_part = std::min(len, buffer_size_ - head);
    std::memcpy(data, buffer_ + 8 + head, first_part);
    if (len > first_part) {
        std::memcpy(static_cast<char*>(data) + first_part, buffer_ + 8, len - first_part);
    }

    head_->store((head + len) % buffer_size_, std::memory_order_release);
    return true;
}



// -------------------------------------------------------------------------------------------
#include "RingBuffer.h"
#include <iostream>
#include <thread>

void writer(RingBuffer& rb) {
    const char* message = "Hello, RingBuffer!";
    for (int i = 0; i < 10; ++i) {
        if (rb.write(message, std::strlen(message) + 1)) {
            std::cout << "Writer: wrote message" << std::endl;
        } else {
            std::cout << "Writer: buffer full" << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void reader(RingBuffer& rb) {
    char buffer[256];
    for (int i = 0; i < 10; ++i) {
        if (rb.read(buffer, sizeof(buffer))) {
            std::cout << "Reader: read message: " << buffer << std::endl;
        } else {
            std::cout << "Reader: buffer empty" << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
    }
}

int main() {
    const char* shm_name = "/my_ringbuffer";
    size_t buffer_size = 1024;

    RingBuffer rb(shm_name, buffer_size);

    std::thread writer_thread(writer, std::ref(rb));
    std::thread reader_thread(reader, std::ref(rb));

    writer_thread.join();
    reader_thread.join();

    return 0;
}
// -------------------------------------------------------------------------------------------















}  // namespace internal

}  // namespace osadaptor::ipc







