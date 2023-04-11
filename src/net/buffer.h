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

class Buffer : public muduo::Copyable {
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;

    explicit Buffer(size_t initialSize = kInitialSize)
        : buffer_(kCheapPrepend + initialSize),
          readerIndex_(kCheapPrepend),
          writerIndex_(kCheapPrepend)
    {
        //
    }

    void swap(Buffer& rhs){

    }
    /** 函数后 const 表示不改变类成员函数，为只读函数 */
    size_t readableBytes() const {}
    size_t writableBytes() const {}
    size_t prependableBytes() const {}
    const char* peek() const {}

    const char* findCRLF() const {}

    const char* findCRLF(const char* start) const {}

    const char* findEOL() const {}

    const char* findEOL(const char* start) const {}
    /** retrieve： 取回 */
    void retrieve(size_t len) {}

    void retrieveUntil(const char* end) {}

    void retrieveInt64() {}

    void retrieveInt32() {}

    void retrieveInt16() {}

    void retrieveInt8() {}

    void retrieveAll() {}

    string retrieveAllAsString() {}

    string retrieveAsString() {}

    StringPiece toStringPiece() const {}

    void append(const StringPiece& str) {}

    void append(const char* data, size_t len) {}

    void append(const void* data, size_t len) {}

    void ensureWritableBytes(size_t len) {}


    /**
     * 这两个函数是同名函数且函数签名（函数名、参数列表和 const 限定符）不同，因此它们可以在同一个类中共存。
    注意到第一个函数 char* beginWrite() {} 没有 const 限定符，因此它表示的是一个非 const 的成员函数，该函数可以修改类中的成员变量。
    而第二个函数 const char* beginWrite() const {} 带有 const 限定符，意味着该函数是一个 const 成员函数，不能修改类中的成员变量。

    在类中，非 const 成员函数可以调用任何成员函数，包括 const 成员函数。但是 const 成员函数只能调用其他的 const 成员函数，因为如果它们
    调用了非 const 成员函数，就会导致非 const 成员函数修改了类中的成员变量，违反了 const 成员函数的 const 特性。

    因此，在这种情况下，两个函数的作用是不同的，它们的使用取决于你需要进行怎样的操作。如果你需要在输出数据之前修改类中的成员变量，
    则使用非 const 成员函数 char* beginWrite() {}，否则使用 const 成员函数 const char* beginWrite() const {}。*/
    char* beginWrite() {}

    const char* beginWrite() const {}

    void hasWritten(size_t len) {}

    void unwrite(size_t len) {}

    void appendInt64(int64_t x) {}
    void appendInt32(int32_t x) {}
    void appendInt16(int16_t x) {}
    void appendInt8(int8_t x) {}

    int64_t readInt64() {}
    int32_t readInt32() {}
    int16_t readInt16() {}
    int8_t readInt8() {}

    int64_t peekInt64() const {}
    int32_t peekInt32() const {}
    int16_t peekInt16() const {}
    int8_t peekInt8() const {}

    void prependInt64(int64_t x) {}
    void prependInt32(int32_t x) {}
    void prependInt16(int16_t x) {}
    void prependInt8(int8_t x) {}

    void prepend(const void* data, size_t len) {}

    void shrink(size_t reserve) {}

    size_t internalCapacity() {}
    /** 实现在 buffer.cpp中 */
    ssize_t readFd(int fd, int* savedErrno);
private:
    char* begin() {}
    const char* begin() const {}

    void makeSpace(size_t len) {}



private:
    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;

    static const char KCRLF[];  /** CRLF: 回车换行 */
};
} // namespace muduo::net





#endif //LIBZV_BUFFER_H
