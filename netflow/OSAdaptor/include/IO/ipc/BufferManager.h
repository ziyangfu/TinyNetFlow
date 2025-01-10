//
// Created by fzy on 2025/1/8.
//

/*!
 * 当 Buffelist 中的 BufferSlice 数量大于1时，分配和回收会分别操作不同的 BufferSlice，互不干扰。但并发分配，或并发回收会存在竞态。
分配操作需要通过对 BufferListHeader 中的head进行 CAS 操作避免并发冲突。
回收操作需要通过对 BufferListHeader 中的tail进行 CAS 操作避免并发冲突

c++语言层面提供了cas操作,在头文件#include< atomic>中

compare_exchange_weak
compare_exchange_strong
 * */

#ifndef TINYNETFLOW_BUFFERMANAGER_H
#define TINYNETFLOW_BUFFERMANAGER_H

#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <queue>
#include <algorithm>
#include <system_error>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <future>


namespace shmipc {

enum class MemMapType {
    kDevShmFile,
    kMemFd
};

struct BufferSlice {
    uint32_t cap;
    uint32_t size;
    uint32_t data_start;
    uint32_t next;
    uint32_t flags;
    uint8_t* data;
    uint32_t offset_in_shm;
    bool is_from_shm;

    void Reset() {
        size = 0;
        flags = 0;
    }

    bool HasNext() const {
        return flags & hasNextBufferFlag;
    }

    uint32_t NextBufferOffset() const {
        return next;
    }

    void SetInUsed() {
        flags |= sliceInUsedFlag;
    }

    void ClearFlag() {
        flags = 0;
    }

    void LinkNext(uint32_t next_offset) {
        next = next_offset;
        flags |= hasNextBufferFlag;
    }
};



struct BufferList {
    std::atomic<int32_t> size;
    std::atomic<uint32_t> cap;
    std::atomic<uint32_t> head;
    std::atomic<uint32_t> tail;
    std::atomic<uint32_t> cap_per_buffer;
    std::atomic<int32_t> counter;
    std::vector<uint8_t> buffer_region;
    uint32_t buffer_region_offset_in_shm;
    uint32_t offset_in_shm;

    BufferList(uint32_t buffer_num, uint32_t cap_per_buffer, std::vector<uint8_t>& mem, uint32_t offset_in_mem)
            : size(buffer_num),
              cap(buffer_num),
              head(0),
              tail((buffer_num - 1) * (cap_per_buffer + bufferHeaderSize)),
              cap_per_buffer(cap_per_buffer),
              counter(0),
              buffer_region_offset_in_shm(offset_in_mem + bufferListHeaderSize),
              offset_in_shm(offset_in_mem) {
        buffer_region = std::vector<uint8_t>(mem.begin() + offset_in_mem + bufferListHeaderSize,
                                             mem.begin() + offset_in_mem + countBufferListMemSize(buffer_num, cap_per_buffer));

        uint32_t current = 0, next = 0;
        for (uint32_t i = 0; i < buffer_num; ++i) {
            next = current + cap_per_buffer + bufferHeaderSize;
            BufferHeader(buffer_region.data() + current).SetCap(cap_per_buffer);
            BufferHeader(buffer_region.data() + current).SetSize(0);
            BufferHeader(buffer_region.data() + current).SetDataStart(0);
            if (i < buffer_num - 1) {
                BufferHeader(buffer_region.data() + current).LinkNext(next);
                BufferHeader(buffer_region.data() + current).SetHasNext(true);
            }
            current = next;
        }
        BufferHeader(buffer_region.data() + tail).ClearFlag();
    }

    std::unique_ptr<BufferSlice> Pop() {
        uint32_t old_head = head.load();
        int32_t remain = size.fetch_sub(1);
        if (remain <= 0) {
            size.fetch_add(1);
            return nullptr;
        }
        for (int i = 0; i < 200; ++i) {
            BufferHeader bh(buffer_region.data() + old_head);
            if (bh.HasNext()) {
                if (head.compare_exchange_weak(old_head, bh.NextBufferOffset())) {
                    bh.ClearFlag();
                    bh.SetInUsed();
                    counter.fetch_add(1);
                    return std::make_unique<BufferSlice>(
                            bh.GetCap(), bh.GetSize(), bh.GetDataStart(), bh.NextBufferOffset(), bh.GetFlags(),
                            buffer_region.data() + old_head + bufferHeaderSize, old_head + buffer_region_offset_in_shm, true);
                }
            } else {
                if (size.load() <= 1) {
                    size.fetch_add(1);
                    return nullptr;
                }
            }
            old_head = head.load();
        }
        size.fetch_add(1);
        return nullptr;
    }

    void Push(BufferSlice* buffer) {
        buffer->Reset();
        for (;;) {
            uint32_t old_tail = tail.load();
            uint32_t new_tail = buffer->offset_in_shm - buffer_region_offset_in_shm;
            if (tail.compare_exchange_weak(old_tail, new_tail)) {
                BufferHeader(buffer_region.data() + old_tail).LinkNext(new_tail);
                size.fetch_add(1);
                counter.fetch_sub(1);
                return;
            }
        }
    }

    int Remain() const {
        return size.load() - 1;
    }
};

struct BufferManager {
    std::vector<std::unique_ptr<BufferList>> lists;
    std::vector<uint8_t> mem;
    uint32_t min_slice_size;
    uint32_t max_slice_size;
    std::string path;
    std::atomic<int32_t> ref_count;
    MemMapType mmap_map_type;
    int mem_fd;

    BufferManager(const std::string& path, std::vector<uint8_t>& mem, uint32_t min_slice_size, uint32_t max_slice_size)
            : path(path),
              mem(mem),
              min_slice_size(min_slice_size),
              max_slice_size(max_slice_size),
              ref_count(1),
              mmap_map_type(MemMapType::kDevShmFile),
              mem_fd(-1) {}

    uint32_t RemainSize() const {
        uint32_t result = 0;
        for (const auto& list : lists) {
            int remain = list->size.load() * list->cap_per_buffer.load();
            if (remain > 0) {
                result += remain;
            }
        }
        return result;
    }

    std::unique_ptr<BufferSlice> AllocShmBuffer(uint32_t size) {
        if (size <= max_slice_size) {
            for (auto& list : lists) {
                if (size <= list->cap_per_buffer.load()) {
                    auto buf = list->Pop();
                    if (buf) {
                        return buf;
                    }
                }
            }
        }
        return nullptr;
    }

    int64_t AllocShmBuffers(std::vector<std::unique_ptr<BufferSlice>>& slices, uint32_t size) {
        int64_t alloc_size = 0;
        int64_t remain = size;
        for (auto it = lists.rbegin(); it != lists.rend() && remain > 0; ++it) {
            while (remain > 0) {
                auto buf = (*it)->Pop();
                if (!buf) {
                    break;
                }
                slices.push_back(std::move(buf));
                alloc_size += buf->cap;
                remain -= buf->cap;
            }
        }
        return alloc_size;
    }

    void RecycleBuffer(BufferSlice* slice) {
        if (!slice) {
            return;
        }
        if (slice->is_from_shm) {
            for (auto& list : lists) {
                if (slice->cap == list->cap_per_buffer.load()) {
                    list->Push(slice);
                    break;
                }
            }
        }
        PutBackBufferSlice(slice);
    }

    void RecycleBuffers(BufferSlice* slice) {
        if (!slice) {
            return;
        }
        if (slice->is_from_shm) {
            while (true) {
                if (!slice->HasNext()) {
                    RecycleBuffer(slice);
                    return;
                }
                uint32_t next_slice_offset = slice->NextBufferOffset();
                RecycleBuffer(slice);
                auto next_slice = ReadBufferSlice(next_slice_offset);
                if (!next_slice) {
                    std::cerr << "bufferManager recycleBuffers readBufferSlice failed" << std::endl;
                    return;
                }
                slice = next_slice.release();
            }
        }
    }

    int SliceSize() const {
        int size = 0;
        for (const auto& list : lists) {
            size += list->size.load();
        }
        return size;
    }

    std::unique_ptr<BufferSlice> ReadBufferSlice(uint32_t offset) {
        if (offset + bufferHeaderSize >= mem.size()) {
            std::cerr << "broken share memory. readBufferSlice unexpected offset: " << offset
                      << " buffers cap: " << mem.size() << std::endl;
            return nullptr;
        }
        uint32_t buf_cap = *reinterpret_cast<uint32_t*>(mem.data() + offset + bufferCapOffset);
        uint32_t buf_end_offset = offset + bufferHeaderSize + buf_cap;
        if (buf_end_offset > mem.size()) {
            std::cerr << "broken share memory. readBufferSlice unexpected bufferEndOffset: " << buf_end_offset
                      << " bufferStartOffset: " << offset << " buffers cap: " << mem.size() << std::endl;
            return nullptr;
        }
        return std::make_unique<BufferSlice>(
                buf_cap, 0, 0, 0, 0, mem.data() + offset + bufferHeaderSize, offset, true);
    }

    void Unmap() {
        // Spin 5s to check if all buffers are returned, if timeout, we still force unmap TODO: ?
        for (int i = 0; i < 50; ++i) {
            if (CheckBufferReturned()) {
                std::cout << "all buffer returned before unmap" << std::endl;
                break;
            }
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
        if (munmap(mem.data(), mem.size()) != 0) {
            std::cerr << "bufferManager unmap error: " << strerror(errno) << std::endl;
        }

        if (mmap_map_type == MemMapType::kDevShmFile) {
            if (std::remove(path.c_str()) != 0) {
                std::cerr << "bufferManager remove file: " << path << " failed, error=" << strerror(errno) << std::endl;
            } else {
                std::cout << "bufferManager removed file: " << path << std::endl;
            }
        }

        if (mmap_map_type == MemMapType::kMemFd) {
            if (close(mem_fd) != 0) {
                std::cerr << "bufferManager close fd: " << mem_fd << " failed, error=" << strerror(errno) << std::endl;
            } else {
                std::cout << "bufferManager close fd: " << mem_fd << std::endl;
            }
        }
    }

    bool CheckBufferReturned() const {
        for (const auto& list : lists) {
            if (list->size.load() != list->cap.load()) {
                return false;
            }
            if (list->counter.load() != 0) {
                return false;
            }
        }
        return true;
    }
};

struct BufferHeader {
    uint32_t cap;
    uint32_t size;
    uint32_t data_start;
    uint32_t next;
    uint32_t flags;

    BufferHeader(uint8_t* data)
            : cap(*reinterpret_cast<uint32_t*>(data + bufferCapOffset)),
              size(*reinterpret_cast<uint32_t*>(data + bufferSizeOffset)),
              data_start(*reinterpret_cast<uint32_t*>(data + bufferDataStartOffset)),
              next(*reinterpret_cast<uint32_t*>(data + nextBufferOffset)),
              flags(*reinterpret_cast<uint32_t*>(data + bufferFlagOffset)) {}

    void SetCap(uint32_t cap) {
        *reinterpret_cast<uint32_t*>(data + bufferCapOffset) = cap;
    }

    void SetSize(uint32_t size) {
        *reinterpret_cast<uint32_t*>(data + bufferSizeOffset) = size;
    }

    void SetDataStart(uint32_t data_start) {
        *reinterpret_cast<uint32_t*>(data + bufferDataStartOffset) = data_start;
    }

    void LinkNext(uint32_t next) {
        *reinterpret_cast<uint32_t*>(data + nextBufferOffset) = next;
    }

    void SetHasNext(bool has_next) {
        if (has_next) {
            flags |= hasNextBufferFlag;
        } else {
            flags &= ~hasNextBufferFlag






#endif //TINYNETFLOW_BUFFERMANAGER_H
