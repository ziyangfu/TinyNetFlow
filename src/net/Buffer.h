//
// Created by fzy on 23-5-17.
//

#ifndef TINYNETFLOW_BUFFER_H
#define TINYNETFLOW_BUFFER_H

#include <algorithm>
#include <vector>

#include <assert.h>
#include <string.h>



namespace netflow::net {
/** 非阻塞IO必备， 包括读buffer和写buffer */
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

class Buffer {
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;

    Buffer(size_t initialSize = kInitialSize)
        : buffer_(kCheapPrepend + initialSize),
          readerIndex_(kCheapPrepend),
          writerIndex_(kCheapPrepend)
    {
        //
    }
    ~Buffer();

    void swap(Buffer& rhs){}

    size_t readableBytes() const {}
    size_t writableBytes() const {}
    size_t prependableBytes() const {}

    const char* peek() const {}

    const char* findCRLF(){}
    const char* findCRLF(const char* start){}

    const char* findEOL(){}
    const char* findEOL(const char* start){}

    /** 取回 */
    void retrieve(size_t len) {}
    void retrieveUntil(const char* end){}

    void retrieveInt64(){}
    void retrieveInt32(){}
    void retrieveInt16(){}
    void retrieveInt8(){}

    void retrieveAll(){}

    std::string retrieveAllAsString(){}
    std::string retrieveAsString(){}

    void append(const char* data, size_t len){}
    void append(const void* data, size_t len){}

    void ensureWritableBytes(size_t len){}

    char* beginWrite(){}
    const char* beginWrite() const {}
    void hasWritten(size_t len){}
    void unwrite(size_t len){}

    void appendInt64(int64_t x){}
    void appendInt32(int32_t x){}
    void appendInt16(int16_t x){}
    void appendInt8(int8_t x){}


    int64_t readInt64(){}
    int32_t readInt32(){}
    int16_t readInt16(){}
    int8_t readInt8(){}

    int64_t peekInt64() const {}
    int32_t peekInt32() const {}
    int16_t peekInt16() const {}
    int8_t peekInt8() const {}

    void prependInt64(int64_t x){}
    void prependInt32(int32_t x){}
    void prependInt16(int16_t x){}
    void prependInt8(int8_t x){}

    void prepend(const void* data, size_t len) {}

    void shrink(size_t reserve){}

    size_t getInternalCapacity() const {}

    ssize_t readFd(int fd, int* savedErrno);  /** 在 Buffer.cpp中实现 */
private:
    char* begin()
    { return &*buffer_.begin(); }

    const char* begin() const
    { return &*buffer_.begin(); }

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

    static const char kCRLF[]; /** 这个作用是什么？ */

};
} // namespace netflow::net



#endif //TINYNETFLOW_BUFFER_H
