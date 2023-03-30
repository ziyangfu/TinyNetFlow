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

    void reset() { cur_ = data_; }
    void bzero() { memZero(data_, sizeof data_); }

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

class LogStream : noncopyable
{
    typedef LogStream self;
public:
    typedef detail::FixedBuffer<detail::kSmallBuffer> Buffer;

    self& operator<<(bool v)
    {
        buffer_.append(v ? "1" : "0", 1);
        return *this;
    }

    self& operator<<(short);
    self& operator<<(unsigned short);
    self& operator<<(int);
    self& operator<<(unsigned int);
    self& operator<<(long);
    self& operator<<(unsigned long);
    self& operator<<(long long);
    self& operator<<(unsigned long long);

    self& operator<<(const void*);

    self& operator<<(float v)
    {
        *this << static_cast<double>(v);
        return *this;
    }
    self& operator<<(double);
    // self& operator<<(long double);

    self& operator<<(char v)
    {
        buffer_.append(&v, 1);
        return *this;
    }

    // self& operator<<(signed char);
    // self& operator<<(unsigned char);

    self& operator<<(const char* str)
    {
        if (str)
        {
            buffer_.append(str, strlen(str));
        }
        else
        {
            buffer_.append("(null)", 6);
        }
        return *this;
    }

    self& operator<<(const unsigned char* str)
    {
        return operator<<(reinterpret_cast<const char*>(str));
    }

    self& operator<<(const string& v)
    {
        buffer_.append(v.c_str(), v.size());
        return *this;
    }

    self& operator<<(const StringPiece& v)
    {
        buffer_.append(v.data(), v.size());
        return *this;
    }

    self& operator<<(const Buffer& v)
    {
        *this << v.toStringPiece();
        return *this;
    }

    void append(const char* data, int len) { buffer_.append(data, len); }
    const Buffer& buffer() const { return buffer_; }
    void resetBuffer() { buffer_.reset(); }

private:
    void staticCheck();

    template<typename T>
    void formatInteger(T);

    Buffer buffer_;

    static const int kMaxNumericSize = 48;
};

class Fmt // : noncopyable
{
public:
    template<typename T>
    Fmt(const char* fmt, T val);

    const char* data() const { return buf_; }
    int length() const { return length_; }

private:
    char buf_[32];
    int length_;
};

inline LogStream& operator<<(LogStream& s, const Fmt& fmt)
{
    s.append(fmt.data(), fmt.length());
    return s;
}

// Format quantity n in SI units (k, M, G, T, P, E).
// The returned string is atmost 5 characters long.
// Requires n >= 0
string formatSI(int64_t n);

// Format quantity n in IEC (binary) units (Ki, Mi, Gi, Ti, Pi, Ei).
// The returned string is atmost 6 characters long.
// Requires n >= 0
string formatIEC(int64_t n);


} // muduo

#endif //LIBZV_LOG_STREAM_H
