//
// Created by fzy on 23-3-13.
//

/*! 缓冲区 非阻塞必备 */
#ifndef LIBZV_BUFFER_H
#define LIBZV_BUFFER_H

#include "../base/copyable.h"
#include "../base/string_piece.h"
#include "../base/types.h"

#include "endian.h"

#include <algorithm>
#include <vector>
#include <assert.h>
#include <string.h>

namespace muduo::net {
/// A buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
///
/// @code
/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
/// @endcode

class Buffer : public muduo::Copyable {
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;

    explicit Buffer(size_t initialSize = kInitialSize)
        : buffer_(kCheapPrepend + initialSize),
          readerIndex_(kCheapPrepend),
          writerIndex_(kCheapPrepend)
    {
        assert(readableBytes() == 0);
        assert(writableBytes() == initialSize);
        assert(prependableBytes() == kCheapPrepend);
    }

    void swap(Buffer& rhs){
        buffer_.swap(rhs.buffer_);
        std::swap(readerIndex_, rhs.readerIndex_);
        std::swap(writerIndex_, rhs.writerIndex_);
    }
    /** 函数后 const 表示不改变类成员函数，为只读函数 */
    size_t readableBytes() const {  return writerIndex_ - readerIndex_;  }
    size_t writableBytes() const { return buffer_.size() - writerIndex_; }
    size_t prependableBytes() const { return readerIndex_; }
    const char* peek() const { return begin() + readerIndex_; }

    const char* findCRLF() const {
        // FIXME: replace with memmem()?
        const char* crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF+2);
        return crlf == beginWrite() ? NULL : crlf;
    }

    const char* findCRLF(const char* start) const {
        assert(peek() <= start);
        assert(start <= beginWrite());
        // FIXME: replace with memmem()?
        const char* crlf = std::search(start, beginWrite(), kCRLF, kCRLF+2);
        return crlf == beginWrite() ? NULL : crlf;
    }

    const char* findEOL() const {
        const void* eol = memchr(peek(), '\n', readableBytes());
        return static_cast<const char*>(eol);
    }

    const char* findEOL(const char* start) const {
        assert(peek() <= start);
        assert(start <= beginWrite());
        const void* eol = memchr(start, '\n', beginWrite() - start);
        return static_cast<const char*>(eol);
    }
    /** retrieve： 取回 */
    void retrieve(size_t len) {
        assert(len <= readableBytes());
        if (len < readableBytes())
        {
            readerIndex_ += len;
        }
        else
        {
            retrieveAll();
        }
    }

    void retrieveUntil(const char* end) {
        assert(peek() <= end);
        assert(end <= beginWrite());
        retrieve(end - peek());
    }

    void retrieveInt64() { retrieve(sizeof(int64_t)); }

    void retrieveInt32() {  retrieve(sizeof(int32_t)); }

    void retrieveInt16() { retrieve(sizeof(int16_t)); }

    void retrieveInt8() { retrieve(sizeof(int8_t)); }

    void retrieveAll() {
        readerIndex_ = kCheapPrepend;
        writerIndex_ = kCheapPrepend;
    }

    string retrieveAllAsString() {
        return retrieveAsString(readableBytes());
    }

    string retrieveAsString(size_t len) {
        assert(len <= readableBytes());
        string result(peek(), len);
        retrieve(len);
        return result;
    }

    StringPiece toStringPiece() const {
        return StringPiece(peek(), static_cast<int>(readableBytes()));
    }

    void append(const StringPiece& str) {
        append(str.data(), str.size());
    }

    void append(const char* data, size_t len) {
        ensureWritableBytes(len);
        std::copy(data, data+len, beginWrite());
        hasWritten(len);
    }

    void append(const void* data, size_t len) {
        append(static_cast<const char*>(data), len);
    }

    void ensureWritableBytes(size_t len) {
        if (writableBytes() < len)
        {
            makeSpace(len);
        }
        assert(writableBytes() >= len);
    }


    /**
     * 这两个函数是同名函数且函数签名（函数名、参数列表和 const 限定符）不同，因此它们可以在同一个类中共存。
    注意到第一个函数 char* beginWrite() {} 没有 const 限定符，因此它表示的是一个非 const 的成员函数，该函数可以修改类中的成员变量。
    而第二个函数 const char* beginWrite() const {} 带有 const 限定符，意味着该函数是一个 const 成员函数，不能修改类中的成员变量。

    在类中，非 const 成员函数可以调用任何成员函数，包括 const 成员函数。但是 const 成员函数只能调用其他的 const 成员函数，因为如果它们
    调用了非 const 成员函数，就会导致非 const 成员函数修改了类中的成员变量，违反了 const 成员函数的 const 特性。

    因此，在这种情况下，两个函数的作用是不同的，它们的使用取决于你需要进行怎样的操作。如果你需要在输出数据之前修改类中的成员变量，
    则使用非 const 成员函数 char* beginWrite() {}，否则使用 const 成员函数 const char* beginWrite() const {}。*/
    char* beginWrite() {  return begin() + writerIndex_;  }

    const char* beginWrite() const {  return begin() + writerIndex_;  }

    void hasWritten(size_t len) {
        assert(len <= writableBytes());
        writerIndex_ += len;
    }

    void unwrite(size_t len) {
        assert(len <= readableBytes());
        writerIndex_ -= len;
    }

    void appendInt64(int64_t x) {
        int64_t be64 = sockets::hostToNetwork64(x);
        append(&be64, sizeof be64);
    }
    void appendInt32(int32_t x) {
        int32_t be32 = sockets::hostToNetwork32(x);
        append(&be32, sizeof be32);
    }
    void appendInt16(int16_t x) {
        int16_t be16 = sockets::hostToNetwork16(x);
        append(&be16, sizeof be16);
    }
    void appendInt8(int8_t x) {
        append(&x, sizeof x);
    }
    ///
    /// Read int64_t from network endian
    ///
    /// Require: buf->readableBytes() >= sizeof(int32_t)
    int64_t readInt64() {
        int64_t result = peekInt64();
        retrieveInt64();
        return result;
    }
    int32_t readInt32() {
        int32_t result = peekInt32();
        retrieveInt32();
        return result;
    }
    int16_t readInt16() {
        int16_t result = peekInt16();
        retrieveInt16();
        return result;
    }
    int8_t readInt8() {
        int8_t result = peekInt8();
        retrieveInt8();
        return result;
    }
    ///
    /// Peek int64_t from network endian
    ///
    /// Require: buf->readableBytes() >= sizeof(int64_t)
    int64_t peekInt64() const {
        assert(readableBytes() >= sizeof(int64_t));
        int64_t be64 = 0;
        ::memcpy(&be64, peek(), sizeof be64);
        return sockets::networkToHost64(be64);
    }
    int32_t peekInt32() const {
        assert(readableBytes() >= sizeof(int32_t));
        int32_t be32 = 0;
        ::memcpy(&be32, peek(), sizeof be32);
        return sockets::networkToHost32(be32);
    }
    int16_t peekInt16() const {
        assert(readableBytes() >= sizeof(int16_t));
        int16_t be16 = 0;
        ::memcpy(&be16, peek(), sizeof be16);
        return sockets::networkToHost16(be16);
    }
    int8_t peekInt8() const {
        assert(readableBytes() >= sizeof(int8_t));
        int8_t x = *peek();
        return x;
    }
    ///
    /// Prepend int64_t using network endian
    ///
    void prependInt64(int64_t x) {
        int64_t be64 = sockets::hostToNetwork64(x);
        prepend(&be64, sizeof be64);
    }
    void prependInt32(int32_t x) {
        int32_t be32 = sockets::hostToNetwork32(x);
        prepend(&be32, sizeof be32);
    }
    void prependInt16(int16_t x) {
        int16_t be16 = sockets::hostToNetwork16(x);
        prepend(&be16, sizeof be16);
    }
    void prependInt8(int8_t x) {
        prepend(&x, sizeof x);
    }

    void prepend(const void* data, size_t len) {
        assert(len <= prependableBytes());
        readerIndex_ -= len;
        const char* d = static_cast<const char*>(data);
        std::copy(d, d+len, begin()+readerIndex_);
    }

    void shrink(size_t reserve) {
        // FIXME: use vector::shrink_to_fit() in C++ 11 if possible.
        Buffer other;
        other.ensureWritableBytes(readableBytes()+reserve);
        other.append(toStringPiece());
        swap(other);
    }

    size_t internalCapacity() {
        return buffer_.capacity();
    }
    /** 实现在 buffer.cpp中 */
    /// Read data directly into buffer.
    ///
    /// It may implement with readv(2)
    /// @return result of read(2), @c errno is saved
    ssize_t readFd(int fd, int* savedErrno);
private:
    char* begin()
    { return &*buffer_.begin(); }

    const char* begin() const
    { return &*buffer_.begin(); }
    /** 若可写字节不够，则扩容 */
    void makeSpace(size_t len)
    {
        if (writableBytes() + prependableBytes() < len + kCheapPrepend)
        {
            // FIXME: move readable data
            buffer_.resize(writerIndex_+len);
        }
        else
        {
            // move readable data to the front, make space inside buffer
            assert(kCheapPrepend < readerIndex_);
            size_t readable = readableBytes();
            std::copy(begin()+readerIndex_,
                      begin()+writerIndex_,
                      begin()+kCheapPrepend);
            readerIndex_ = kCheapPrepend;
            writerIndex_ = readerIndex_ + readable;
            assert(readable == readableBytes());
        }
    }
private:
    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;

    static const char kCRLF[];  /** CRLF: 回车换行 */
};
} // namespace muduo::net





#endif //LIBZV_BUFFER_H
