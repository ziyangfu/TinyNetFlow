//
// Created by fzy on 2025/1/8.
//

/** 高并发场景下，使用Buffer，大量切片
 *  低并发或者无并发场景下，可以考虑直接使用ringbuffer
 * */

#ifndef TINYNETFLOW_BUFFER_H
#define TINYNETFLOW_BUFFER_H

#include <cstdint>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <system_error>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <algorithm>
#include <filesystem>
#include <future>
#include <atomic>

namespace shmipc {

class Stream;
class BufferManager;
class BufferSlice;
class SliceList;

class BufferWriter {
public:
    virtual ~BufferWriter() = default;

    virtual int Len() const = 0;
    virtual std::error_code WriteByte(uint8_t b) = 0;
    virtual std::pair<int, std::error_code> WriteBytes(const std::vector<uint8_t>& data) = 0;
    virtual std::pair<std::vector<uint8_t>, std::error_code> Reserve(size_t size) = 0;
    virtual std::error_code WriteString(const std::string& str) = 0;
};

class BufferReader {
public:
    virtual ~BufferReader() = default;

    virtual int Len() const = 0;
    virtual std::pair<uint8_t, std::error_code> ReadByte() = 0;
    virtual std::pair<std::vector<uint8_t>, std::error_code> ReadBytes(size_t size) = 0;
    virtual std::pair<std::vector<uint8_t>, std::error_code> Peek(size_t size) = 0;
    virtual std::pair<int, std::error_code> Discard(size_t size) = 0;
    virtual void ReleasePreviousRead() = 0;
    virtual std::pair<std::string, std::error_code> ReadString(size_t size) = 0;
};

class BufferSlice {
public:
    BufferSlice(uint32_t offset_in_shm, std::vector<uint8_t> data, bool is_from_shm)
            : offset_in_shm_(offset_in_shm), data_(std::move(data)), is_from_shm_(is_from_shm), read_index_(0), write_index_(0) {}

    size_t Size() const {
        return write_index_ - read_index_;
    }

    bool HasNext() const {
        // Implementation to check if there is a next buffer slice
        return false;
    }

    uint32_t NextBufferOffset() const {
        // Implementation to get the next buffer offset
        return 0;
    }

    std::pair<std::vector<uint8_t>, std::error_code> Reserve(size_t size) {
        if (write_index_ + size > data_.size()) {
            return {{}, std::make_error_code(std::errc::no_buffer_space)};
        }
        std::vector<uint8_t> ret(data_.data() + write_index_, data_.data() + write_index_ + size);
        write_index_ += size;
        return {ret, std::error_code()};
    }

    std::pair<int, std::error_code> Write(const std::vector<uint8_t>& data) {
        if (write_index_ + data.size() > data_.size()) {
            return {0, std::make_error_code(std::errc::no_buffer_space)};
        }
        std::memcpy(data_.data() + write_index_, data.data(), data.size());
        write_index_ += data.size();
        return {static_cast<int>(data.size()), std::error_code()};
    }

    std::pair<std::vector<uint8_t>, std::error_code> Read(size_t size) {
        if (read_index_ + size > write_index_) {
            return {{}, std::make_error_code(std::errc::no_buffer_space)};
        }
        std::vector<uint8_t> ret(data_.data() + read_index_, data_.data() + read_index_ + size);
        read_index_ += size;
        return {ret, std::error_code()};
    }

    std::pair<std::vector<uint8_t>, std::error_code> Peek(size_t size) {
        if (read_index_ + size > write_index_) {
            return {{}, std::make_error_code(std::errc::no_buffer_space)};
        }
        std::vector<uint8_t> ret(data_.data() + read_index_, data_.data() + read_index_ + size);
        return {ret, std::error_code()};
    }

    int Skip(size_t size) {
        if (read_index_ + size > write_index_) {
            size = write_index_ - read_index_;
        }
        read_index_ += size;
        return static_cast<int>(size);
    }

    void Reset() {
        read_index_ = 0;
        write_index_ = 0;
    }

    bool IsFromShm() const {
        return is_from_shm_;
    }

    uint32_t OffsetInShm() const {
        return offset_in_shm_;
    }

private:
    uint32_t offset_in_shm_;
    std::vector<uint8_t> data_;
    bool is_from_shm_;
    size_t read_index_;
    size_t write_index_;
};

class SliceList {
public:
    SliceList() = default;

    void PushBack(std::unique_ptr<BufferSlice> slice) {
        slices_.push_back(std::move(slice));
    }

    std::unique_ptr<BufferSlice> PopFront() {
        if (slices_.empty()) {
            return nullptr;
        }
        auto front = std::move(slices_.front());
        slices_.pop_front();
        return front;
    }

    BufferSlice* Front() {
        if (slices_.empty()) {
            return nullptr;
        }
        return slices_.front().get();
    }

    BufferSlice* Back() {
        if (slices_.empty()) {
            return nullptr;
        }
        return slices_.back().get();
    }

    size_t Size() const {
        return slices_.size();
    }

    void SplitFromWrite() {
        // Implementation to split from write slice
    }

private:
    std::deque<std::unique_ptr<BufferSlice>> slices_;
};

class BufferManager {
public:
    std::pair<std::unique_ptr<BufferSlice>, std::error_code> AllocShmBuffer(uint32_t size) {
        // Implementation to allocate shared memory buffer
        return {std::make_unique<BufferSlice>(0, std::vector<uint8_t>(size), true), std::error_code()};
    }

    void RecycleBuffer(BufferSlice* slice) {
        // Implementation to recycle buffer
    }

    // Additional methods as needed
};

class LinkedBuffer : public BufferWriter, public BufferReader {
public:
    explicit LinkedBuffer(BufferManager* buffer_manager)
            : buffer_manager_(buffer_manager),
              slice_list_(std::make_unique<SliceList>()),
              pinned_list_(std::make_unique<SliceList>()),
              current_pinned_(false),
              end_stream_(false),
              is_from_shm_(true),
              len_(0) {}

    int Len() const override {
        return len_;
    }

    std::error_code WriteByte(uint8_t b) override {
        if (!slice_list_->Front()) {
            auto err = Alloc(1);
            if (err) {
                return err;
            }
            slice_list_->Front()->Write({&b, 1});
        } else {
            auto [_, err] = slice_list_->Front()->Write({&b, 1});
            if (err) {
                auto err = Alloc(1);
                if (err) {
                    return err;
                }
                slice_list_->Front()->Write({&b, 1});
            }
        }
        len_++;
        return std::error_code();
    }

    std::pair<int, std::error_code> WriteBytes(const std::vector<uint8_t>& data) override {
        if (data.empty()) {
            return {0, std::error_code()};
        }

        int written = 0;
        while (written < data.size()) {
            if (!slice_list_->Front()) {
                auto err = Alloc(static_cast<uint32_t>(data.size() - written));
                if (err) {
                    return {written, err};
                }
            }
            auto [n, err] = slice_list_->Front()->Write({data.data() + written, data.size() - written});
            if (err) {
                auto err = Alloc(static_cast<uint32_t>(data.size() - written));
                if (err) {
                    return {written, err};
                }
            } else {
                written += n;
            }
        }
        len_ += written;
        return {written, std::error_code()};
    }

    std::pair<std::vector<uint8_t>, std::error_code> Reserve(size_t size) override {
        if (!slice_list_->Front()) {
            auto err = Alloc(static_cast<uint32_t>(size));
            if (err) {
                return {{}, err};
            }
        }

        auto [ret, err] = slice_list_->Front()->Reserve(size);
        if (!err) {
            len_ += static_cast<int>(size);
            return {ret, std::error_code()};
        }

        if (slice_list_->Front()->NextBufferOffset()) {
            auto [ret, err] = slice_list_->Front()->Reserve(size);
            if (!err) {
                slice_list_->Front() = slice_list_->Front()->NextBufferOffset();
                len_ += static_cast<int>(size);
                return {ret, std::error_code()};
            }
        }

        auto [buf, err2] = buffer_manager_->AllocShmBuffer(static_cast<uint32_t>(size));
        if (!err2) {
            slice_list_->PushBack(std::move(buf));
        } else {
            size_t alloc_size = size;
            if (alloc_size < kDefaultSingleBufferSize) {
                alloc_size = kDefaultSingleBufferSize;
            }
            slice_list_->PushBack(std::make_unique<BufferSlice>(0, std::vector<uint8_t>(alloc_size), false));
            is_from_shm_ = false;
        }
        slice_list_->Front() = slice_list_->Back();
        len_ += static_cast<int>(size);
        return slice_list_->Front()->Reserve(size);
    }

    std::error_code WriteString(const std::string& str) override {
        auto [n, err] = WriteBytes({reinterpret_cast<const uint8_t*>(str.data()), str.size()});
        return err;
    }

    void Recycle() {
        std::lock_guard<std::mutex> lock(recycle_mux_);
        while (!slice_list_->Empty()) {
            auto slice = slice_list_->PopFront();
            if (slice->IsFromShm()) {
                buffer_manager_->RecycleBuffer(slice.get());
            } else {
                PutBackBufferSlice(slice.get());
            }
        }
        Clean();
    }

    uint32_t RootBufOffset() const {
        return slice_list_->Front()->OffsetInShm();
    }

    BufferReader Done(bool end_stream) {
        end_stream_ = end_stream;
        if (is_from_shm_) {
            for (auto slice = slice_list_->Front(); slice != nullptr; slice = slice->NextBufferOffset()) {
                // slice->Update();
                if (slice == slice_list_->Front()) {
                    break;
                }
            }
            if (slice_list_->Front()->NextBufferOffset()) {
                auto head = slice_list_->SplitFromWrite();
                for (auto slice = head; slice != nullptr;) {
                    auto next = slice->NextBufferOffset();
                    buffer_manager_->RecycleBuffer(slice);
                    slice = next;
                }
            }
        }
        return *this;
    }

    std::vector<std::vector<uint8_t>> UnderlyingData() const {
        std::vector<std::vector<uint8_t>> data;
        for (auto slice = slice_list_->Front(); slice != nullptr; slice = slice->NextBufferOffset()) {
            data.push_back({slice->Data().data() + slice->ReadIndex(), slice->Data().data() + slice->WriteIndex()});
            if (slice == slice_list_->Front()) {
                break;
            }
        }
        return data;
    }

    std::pair<int, std::error_code> Read(uint8_t* p, size_t len) override {
        if (len == 0) {
            return {0, std::error_code()};
        }

        int written = 0;
        if (len_ < 1) {
            auto err = stream_->ReadMore(1);
            if (err) {
                return {0, err};
            }
        }

        for (auto front = slice_list_->Front(); front != nullptr && written < len;) {
            auto [b, err] = front->Read(len - written);
            if (!err) {
                std::memcpy(p + written, b.data(), b.size());
                written += static_cast<int>(b.size());
            } else if (err == std::make_error_code(std::errc::no_buffer_space)) {
                ReadNextSlice();
                front = slice_list_->Front();
            }
        }

        len_ -= written;
        return {written, std::error_code()};
    }

    std::pair<uint8_t, std::error_code> ReadByte() override {
        if (len_ < 1) {
            auto err = stream_->ReadMore(1);
            if (err) {
                return {0, err};
            }
        }

        auto [r, err] = slice_list_->Front()->Read(1);
        if (!err) {
            len_







#endif //TINYNETFLOW_BUFFER_H
