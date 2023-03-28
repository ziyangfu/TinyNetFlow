//
// Created by fzy on 23-3-28.
//
//! 日志流

#ifndef LIBZV_LOG_STREAM_H
#define LIBZV_LOG_STREAM_H

#include "noncopyable.h"
#include "types.h"
#include "string_piece.h"

#include <assert.h>
#include <string.h>  // memcpy

namespace muduo {
namespace detail {

const int kSmallBuffer = 4000;
const int kLargeBuffer = 4000 * 1000;

template<int SIZE>
class FixedBuffer : Noncopyable {
public:
    FixedBuffer()
        : cur_(data_) {
        setCookie(cookieStart);   //! 把指向 cookieStart函数的函数指针传给 cookie_
    }
    ~FixedBuffer() {
        setCookie(cookieEnd);
    }

    void append(const char* buf, size_t len) {
        if (implicit_cast<size_t>(avail()) > len) {   //! 为什么要用 implicit_cast，而不用 static_cast
            memccpy(cur_, buf, len);
            cur_ += len;
        }
    }

    const char* data() const { return data_; }

    int length() const { return static_cast<int>(cur_ - data_); }  //! 当前存入数据的长度

    //! 直接写入数据
    char* current() { return cur_; }
    int avail() const {return static_cast<int>(end() - data_);}   //! 数组总大小
    void add(size_t len) { cur_ += len; }

    //! for GDB 调试
    const char* debugString();
    //! for 单元测试
    string toString() const { return string(data_, length()); }

    void setCookie(void (*cookie)()) { cookie_ = cookie; } //! 传入一个函数指针

    StringPiece toStringPiece() const { return StringPiece(data_, length());}

private:
    const char* end() const { return data_ + sizeof(data_); }  //! 返回指向数组末尾的指针
    static void cookieStart();
    static void cookieEnd();

private:
    void (*cookie_)();  //! cookie_ 是一个指向函数的指针，该函数没有输入参数，也没有返回值
    char data_[SIZE];
    char* cur_;   //! 指向 data_数组中下一个空余位置的指针



};
} // namespace detail

    class log_stream {

    };

} // muduo

#endif //LIBZV_LOG_STREAM_H
