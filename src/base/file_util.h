//
// Created by fzy on 23-3-30.
//
//!  文件操作

#ifndef LIBZV_FILE_UTIL_H
#define LIBZV_FILE_UTIL_H

#include "noncopyable.h"
#include "string_piece.h"
#include <sys/types.h>   //! for off_t, off_t类型用于指示文件的偏移量，常就是long类型

namespace muduo {
namespace FileUtil {

//! 读取小文件 size < 64KB
class ReadSmallFile : Noncopyable {
public:
    ReadSmallFile(StringArg filename);
    ~ReadSmallFile();

    // return errno
    template<typename String>
    int readToString(int maxSize,
                     String* content,
                     int64_t* fileSize,
                     int64_t* modifyTime,
                     int64_t* createTime);

    /// Read at maxium kBufferSize into buf_
    // return errno
    int readToBuffer(int* size);

    const char* buffer() const { return buf_; }

public:
    static const int kBufferSize = 64*1024;
private:
    int fd_;
    int err_;
    char buf_[kBufferSize];
};

template<typename String>
int ReadFile(StringArg filename,   //! 类StringArg 在 string_piece文件内
             int maxSize,
             String* content,
             int64_t* fileSize = NULL,
             int64_t* modifyTime = NULL,
             int64_t* createTime = NULL)
{
    ReadSmallFile file(filename);
    return file.readToString(maxSize, content, fileSize, modifyTime, createTime);
}

//! 非线程安全
class AppendFile : Noncopyable {
public:
    explicit AppendFile(StringArg filename);

    ~AppendFile();

    void append(const char* logline, size_t len);

    void flush();

    off_t writtenBytes() const { return writtenBytes_; }
private:
    size_t write(const char* logline, size_t len);
private:
    FILE* fp_;
    char buffer_[64*1024];
    off_t writtenBytes_;  //! off_t类型用于指示文件的偏移量，常就是long类型
};

} // namespace FileUtil

} // namespace muduo

#endif //LIBZV_FILE_UTIL_H
