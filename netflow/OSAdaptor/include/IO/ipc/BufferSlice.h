//
// Created by fzy on 2025/1/8.
//

#ifndef TINYNETFLOW_BUFFERSLICE_H
#define TINYNETFLOW_BUFFERSLICE_H

#include <iostream>
#include <vector>
#include <memory>
#include <mutex>
#include <cstring>
#include <stdexcept>

namespace shmipc {

constexpr size_t bufferCapOffset = 0;
constexpr size_t bufferSizeOffset = 4;
constexpr size_t bufferDataStartOffset = 8;
constexpr size_t nextBufferOffset = 12;
constexpr size_t bufferFlagOffset = 16;

constexpr uint32_t hasNextBufferFlag = 0x01;
constexpr uint32_t sliceInUsedFlag = 0x02;

class BufferSlice {
public:
    using BufferHeader = std::vector<uint8_t>;

    BufferSlice() : cap(0), start(0), offsetInShm(0), readIndex(0), writeIndex(0), isFromShm(false), nextSlice(nullptr) {}

    BufferSlice* next() const { return nextSlice; }

    static std::shared_ptr<BufferSlice> newBufferSlice(const BufferHeader& header, std::vector<uint8_t>& data, uint32_t offsetInShm, bool isFromShm) {
        auto s = bufferSlicePool.get();
        if (!s) {
            s = std::make_shared<BufferSlice>();
        }
        if (isFromShm && !header.empty()) {
            s->cap = *reinterpret_cast<const uint32_t*>(header.data() + bufferCapOffset);
            s->start = *reinterpret_cast<const uint32_t*>(header.data() + bufferDataStartOffset);
            s->readIndex = static_cast<int>(s->start);
            s->writeIndex = static_cast<int>(s->start + *reinterpret_cast<const uint32_t*>(header.data() + bufferSizeOffset));
        } else {
            s->cap = static_cast<uint32_t>(data.capacity());
        }
        s->bufferHeader = header;
        s->data = data;
        s->offsetInShm = offsetInShm;
        s->isFromShm = isFromShm;
        return s;
    }

    void putBack() {
        isFromShm = false;
        offsetInShm = 0;
        data.clear();
        bufferHeader.clear();
        cap = 0;
        writeIndex = 0;
        readIndex = 0;
        start = 0;
        nextSlice = nullptr;
        bufferSlicePool.release(this);
    }

    uint32_t nextBufferOffset() const {
        return *reinterpret_cast<const uint32_t*>(bufferHeader.data() + nextBufferOffset);
    }

    bool hasNext() const {
        return (bufferHeader[bufferFlagOffset] & hasNextBufferFlag) > 0;
    }

    void clearFlag() {
        bufferHeader[bufferFlagOffset] = 0;
    }

    void setInUsed() {
        bufferHeader[bufferFlagOffset] |= sliceInUsedFlag;
    }

    bool isInUsed() const {
        return (bufferHeader[bufferFlagOffset] & sliceInUsedFlag) > 0;
    }

    void linkNext(uint32_t next) {
        *reinterpret_cast<uint32_t*>(bufferHeader.data() + nextBufferOffset) = next;
        bufferHeader[bufferFlagOffset] |= hasNextBufferFlag;
    }

    void update() {
        if (!bufferHeader.empty()) {
            *reinterpret_cast<uint32_t*>(bufferHeader.data() + bufferSizeOffset) = static_cast<uint32_t>(size());
            *reinterpret_cast<uint32_t*>(bufferHeader.data() + bufferDataStartOffset) = start;
            if (nextSlice) {
                linkNext(nextSlice->offsetInShm);
            }
        }
    }

    void reset() {
        if (!bufferHeader.empty()) {
            *reinterpret_cast<uint32_t*>(bufferHeader.data() + bufferSizeOffset) = 0;
            *reinterpret_cast<uint32_t*>(bufferHeader.data() + bufferDataStartOffset) = 0;
            clearFlag();
        }
        writeIndex = 0;
        readIndex = 0;
        nextSlice = nullptr;
    }

    int size() const {
        return writeIndex - readIndex;
    }

    int remain() const {
        return static_cast<int>(cap) - writeIndex;
    }

    int capacity() const {
        return static_cast<int>(cap);
    }

    std::vector<uint8_t> reserve(int size) {
        if (remain() >= size) {
            int start = writeIndex;
            writeIndex += size;
            return std::vector<uint8_t>(data.begin() + start, data.begin() + writeIndex);
        }
        throw std::runtime_error("No more buffer");
    }

    void prepend() {
        throw std::runtime_error("TODO");
    }

    int append(const std::vector<uint8_t>& data) {
        if (data.empty()) {
            return 0;
        }
        int copySize = std::min(static_cast<int>(data.size()), remain());
        std::copy(data.begin(), data.begin() + copySize, data.begin() + writeIndex);
        writeIndex += copySize;
        return copySize;
    }

    std::vector<uint8_t> read(int size) {
        int unRead = size();
        if (unRead < size) {
            size = unRead;
        }
        std::vector<uint8_t> result(data.begin() + readIndex, data.begin() + readIndex + size);
        readIndex += size;
        return result;
    }

    std::vector<uint8_t> peek(int size) {
        int origin = readIndex;
        auto result = read(size);
        readIndex = origin;
        return result;
    }

    int skip(int size) {
        int unRead = size();
        if (unRead > size) {
            readIndex += size;
            return size;
        }
        readIndex += unRead;
        return unRead;
    }

private:
    BufferHeader bufferHeader;
    std::vector<uint8_t> data;
    uint32_t cap;
    uint32_t start;
    uint32_t offsetInShm;
    int readIndex;
    int writeIndex;
    bool isFromShm;
    BufferSlice* nextSlice;

    static ObjectPool<BufferSlice> bufferSlicePool;
};

template <typename T>
class ObjectPool {
public:
    std::shared_ptr<T> get() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!pool_.empty()) {
            auto obj = std::move(pool_.back());
            pool_.pop_back();
            return obj;
        }
        return std::make_shared<T>();
    }

    void release(T* obj) {
        std::lock_guard<std::mutex> lock(mutex_);
        pool_.emplace_back(obj);
    }

private:
    std::vector<std::shared_ptr<T>> pool_;
    std::mutex mutex_;
};

ObjectPool<BufferSlice> BufferSlice::bufferSlicePool;

class SliceList {
public:
    SliceList() : frontSlice(nullptr), writeSlice(nullptr), backSlice(nullptr), len(0) {}

    BufferSlice* front() const { return frontSlice; }
    BufferSlice* back() const { return backSlice; }
    int size() const { return len; }

    void pushBack(std::shared_ptr<BufferSlice> s) {
        if (!s) {
            return;
        }
        if (len > 0) {
            backSlice->nextSlice = s.get();
        } else {
            frontSlice = s.get();
        }
        backSlice = s.get();
        len++;
    }

    std::shared_ptr<BufferSlice> popFront() {
        auto r = std::shared_ptr<BufferSlice>(frontSlice);
        if (len > 0) {
            len--;
            frontSlice = frontSlice->nextSlice;
        }
        if (len == 0) {
            frontSlice = nullptr;
            backSlice = nullptr;
        }
        return r;
    }

    std::shared_ptr<BufferSlice> splitFromWrite() {
        auto nextListHead = std::shared_ptr<BufferSlice>(writeSlice->nextSlice);
        backSlice = writeSlice;
        backSlice->nextSlice = nullptr;
        int nextListSize = 0;
        for (auto s = nextListHead.get(); s != nullptr; s = s->nextSlice) {
            nextListSize++;
        }
        len -= nextListSize;
        return nextListHead;
    }

private:
    BufferSlice* frontSlice;
    BufferSlice* writeSlice;
    BufferSlice* backSlice;
    int len;
};

} // namespace shmipc

int main() {
    using namespace shmipc;

    // 示例使用
    BufferSlice::BufferHeader header(16, 0);
    std::vector<uint8_t> data(1024, 0);

    auto slice = BufferSlice::newBufferSlice(header, data, 0, false);
    slice->append({1, 2, 3, 4, 5});
    auto readData = slice->read(3);
    for (auto byte : readData) {
        std::cout << static_cast<int>(byte) << " ";
    }
    std::cout << std::endl;

    return 0;
}

#endif //TINYNETFLOW_BUFFERSLICE_H
