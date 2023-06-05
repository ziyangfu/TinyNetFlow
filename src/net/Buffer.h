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
/** 非阻塞IO必备， 包括读buffer和写buffer
 *  核心功能：
 *      1. 数据写入
 *      2. 数据读取
 *      3. 容器扩容
 *      4. 容器缩容
 * 参考： 《UNIX网络编程：套接字API》16章，非阻塞IO
 *      */
/// A buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
///
/// @code
/// +-------------------+------------------+------------------+-------------------
/// | prependable bytes |  readable bytes  |  writable bytes  |                  |
/// |                   |     (CONTENT)    |                  |                  |
/// +-------------------+------------------+------------------+------------------+
/// |                   |                  |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size              capacity
/// @endcode

class Buffer {
public:
    inline static const size_t kCheapPrepend = 8;
    inline static const size_t kInitialSize = 1024;

    Buffer(size_t initialSize = kInitialSize)
        : buffer_(kCheapPrepend + initialSize),  /** 默认空间 size： 1024 + 8 */
          readerIndex_(kCheapPrepend),
          writerIndex_(kCheapPrepend)
    {
        assert(readableBytes() == 0);
        assert(writableBytes() == initialSize);
        assert(prependableBytes() == kCheapPrepend);
    }
    ~Buffer();
    /*!
     * \brief: 交换 Buffer */
    void swap(Buffer& rhs){
        buffer_.swap(rhs.buffer_);
        std::swap(readerIndex_, rhs.readerIndex_);
        std::swap(writerIndex_, rhs.writerIndex_);
    }
    /*!
     * \brief 返回可读(未读)区间大小 */
    size_t readableBytes() const { return writerIndex_ - readerIndex_; }
    /*!
     * \brief 返回可写区间大小 */
    size_t writableBytes() const { return buffer_.size() - writerIndex_; }
    /*!
     * \brief 返回前端预留区间大小 */
    size_t prependableBytes() const { return readerIndex_; }  // TODO: readerIndex_不断移动，前段预留空间不断变化？？？
    /*!
     * \brief 可写区间扩容 */
    void ensureWritableBytes(size_t len){
        if (writableBytes() < len) {
            makeSpace(len);
        }
        assert(writableBytes() >= len);
    }
    /*!
     * \brief 获取可写位置的指针 */
    char* beginWrite(){ return begin() + writerIndex_; }
    const char* beginWrite() const { return begin() +writerIndex_; }
    /*!
     * \brief 已写入 len 大小的数据，writerIndex_向右挪移len长度 */
    void hasWritten(size_t len){
        assert(len <= writableBytes());
        writerIndex_ += len;
    }
    /*!
 * \brief 撤销 len 大小的数据，writerIndex_向左挪移len长度 */
    void unwrite(size_t len){
        assert(len <= writableBytes());
        writerIndex_ -= len;
    }
    /*!
     * \brief 获取可读位置的指针 */
    const char* peek() const { return begin() + readerIndex_; }
    /*!
     * \brief 查找可读区有没有 “\r\n” */
    const char* findCRLF() const {
        const char* crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF+2);
        return crlf == beginWrite() ? nullptr : crlf;
    }
    /*!
     * \brief 从 start开始， 查找start后的可读区间有没有 “\r\n” */
    const char* findCRLF(const char* start) const {
        assert(peek() <= start);
        assert(start <= beginWrite());
        // FIXME: replace with memmem()?
        const char* crlf = std::search(start, beginWrite(), kCRLF, kCRLF+2);
        return crlf == beginWrite() ? NULL : crlf;
    }

    const char* findEOL(){
        /** C 库函数 void *memchr(const void *str, int c, size_t n) 在参数 str 所指向的字符串的
         *  前 n 个字节中搜索第一次出现字符 c（一个无符号字符）的位置。*/
        const void* eol = memchr(peek(), '\n', readableBytes());
        return static_cast<const char*>(eol);
    }
    const char* findEOL(const char* start){
        assert(peek() <= start);
        assert(start <= beginWrite());
        const void* eol = memchr(start, '\n', beginWrite() - start);
        return static_cast<const char*>(eol);
    }

    /*!
     * \brief 读取指定长度的数据时，将readerIndex_向右移动len长度 */
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
    /*!
     * \brief 可读指针移动到 end位置 */
    void retrieveUntil(const char* end){
        assert(peek() <= end);
        assert(end <= beginWrite());
        retrieve(end - peek());
    }
    /*!
     * \brief 可读指针向右移动 sizeof（int64_t）字节 */
    void retrieveInt64() { retrieve(sizeof(int64_t)); }
    void retrieveInt32() { retrieve(sizeof(int32_t)); }
    void retrieveInt16() { retrieve(sizeof(int16_t)); }
    void retrieveInt8() { retrieve(sizeof(int8_t)); }

    void retrieveAll(){
        readerIndex_ = kCheapPrepend;
        writerIndex_ = kCheapPrepend;
    }
    /*!
     * \brief 读取所有可读区的数据 */
    std::string retrieveAllAsString(){
        return retrieveAsString(readableBytes());
    }
    /*!
    * \brief 读取一定长度的可读区数据 */
    std::string retrieveAsString(size_t len){
        assert(len <= readableBytes());
        std::string result(peek(), len);
        retrieve(len);
        return result;
    }
    /**
    StringPiece toStringPiece() const
    {
        return StringPiece(peek(), static_cast<int>(readableBytes()));
    }
     */
    /*!
     * \brief 追加指定大小数据 */
    void append(const char* data, size_t len){
        ensureWritableBytes(len);
        std::copy(data, data+len, beginWrite());
        hasWritten(len);
    }
    void append(const void* data, size_t len){
        append(static_cast<const char*>(data), len);
    }
    /*
     * void append(const StringPiece& str)
      {
        append(str.data(), str.size());
      } */
    /*!
     * \brief 追加int64_t 大小数据
     * 注意： 入参为主机字节序，实际写入到buffer中为网络字节序 */
    void appendInt64(int64_t x){
        int64_t be64 = htobe64(x);  /** 字节序转换， host --> net */
        append(&be64, sizeof be64);
    }
    void appendInt32(int32_t x){
        int32_t be32 = htobe32(x);
        append(&be32, sizeof be32);
    }
    void appendInt16(int16_t x){
        int16_t be16 = htobe16(x);
        append(&be16, sizeof be16);
    }
    void appendInt8(int8_t x){
        append(&x, sizeof x);
    }

    /*!
     * \brief 从网络字节序数据中，读取 int64_t数据
     * 注意：可读区间要大于int64_t大小
     *      取回的数据为主机字节序 */
    int64_t readInt64(){
        int64_t result = peekInt64();
        retrieveInt64();
        return result;
    }
    int32_t readInt32(){
        int32_t result = peekInt32();
        retrieveInt32();
        return result;
    }
    int16_t readInt16(){
        int16_t result = peekInt16();
        retrieveInt16();
        return result;
    }
    int8_t readInt8(){
        int8_t result = peekInt8();
        retrieveInt8();
        return result;
    }

    int64_t peekInt64() const {
        assert(readableBytes() >= sizeof(int64_t));
        int64_t be64 = 0;
        ::memcpy(&be64, peek(), sizeof be64);
        return be64toh(be64);
    }
    int32_t peekInt32() const {
        assert(readableBytes() >= sizeof(int32_t ));
        int32_t be32 = 0;
        ::memcpy(&be32, peek(), sizeof be32);
        return be32toh(be32);
    }
    int16_t peekInt16() const {
        assert(readableBytes() >= sizeof(int16_t));
        int16_t be16 = 0;
        ::memcpy(&be16, peek(), sizeof be16);
        return be16toh(be16);
    }
    int8_t peekInt8() const {
        assert(readableBytes() >= sizeof(int8_t));
        int8_t x = *peek();
        return x;
    }
    /*!
     * \brief 填充前端预留区数据
     * 注意： 入参为主机字节序，实际写入到buffer中为网络字节序 */
    void prependInt64(int64_t x){
        int64_t be64 = htobe64(x);
        prepend(&be64, sizeof be64);
    }
    void prependInt32(int32_t x){
        int32_t be32 = htobe32(x);
        prepend(&be32, sizeof be32);
    }
    void prependInt16(int16_t x){
        int16_t be16 = htobe16(x);
        prepend(&be16, sizeof be16);
    }
    void prependInt8(int8_t x){
        prepend(&x, sizeof x);
    }

    void prepend(const void* data, size_t len) {
        assert(len <= prependableBytes());
        readerIndex_ -= len;
        const char* d = static_cast<const char*>(data);
        std::copy(d, d+len, begin()+readerIndex_);
    }
    /*!
     * \brief 缩容，将多余空间返回给内存 */
    void shrink() {
        buffer_.shrink_to_fit();
    }
    void shrink(size_t reserve){
        //
    }

    size_t getInternalCapacity() const { return buffer_.capacity(); }

    ssize_t readFd(int fd, int* savedErrno);  /** 在 Buffer.cpp中实现 */
private:
    /*!
     * \brief 获取 buffer_容器的起点地址 */
    char* begin()
    { return &*buffer_.begin(); }

    const char* begin() const
    { return &*buffer_.begin(); }
    /*!
     * \brief 当容器空间不够时，容器扩容*/
    void makeSpace(size_t len)
    {
        /** 空间不够，扩容 */
        if (writableBytes() + prependableBytes() < len + kCheapPrepend)
        {
            // FIXME: move readable data
            buffer_.resize(writerIndex_+len);
        }
        else
        {
            /** 空间实际足够，但前面闲置，造成后面不够。因此不扩容，将还未读的空间挪到最初的起始点，腾出后面的空间 */
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

    inline static const char kCRLF[] = "\r\n";  /** C++ 17 */

};
} // namespace netflow::net



#endif //TINYNETFLOW_BUFFER_H
