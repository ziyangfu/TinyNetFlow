//
// Created by fzy on 23-3-30.
//
//! 读写 Gzip 格式的压缩文件

#ifndef LIBZV_GZIP_FILE_H
#define LIBZV_GZIP_FILE_H

#include "string_piece.h"
#include "noncopyable.h"

#include <zlib.h>

namespace muduo {

class GzipFile : Noncopyable {
public:
    GzipFile(GzipFile&& rhs) noexcept
            : file_(rhs.file_)
            {
                    rhs.file_ = NULL;
            }

    ~GzipFile()
    {
        if (file_)
        {
            ::gzclose(file_);
        }
    }

    GzipFile& operator=(GzipFile&& rhs) noexcept
    {
        swap(rhs);
        return *this;
    }

    bool valid() const { return file_ != NULL; }
    void swap(GzipFile& rhs) { std::swap(file_, rhs.file_); }
#if ZLIB_VERNUM >= 0x1240
        bool setBuffer(int size) { return ::gzbuffer(file_, size) == 0; }
#endif

        // return the number of uncompressed bytes actually read, 0 for eof, -1 for error
        //! // 读取指定长度的数据，并返回实际读取的长度，返回 0 表示已经读到文件尾，返回 -1 表示读取失败
        int read(void* buf, int len) { return ::gzread(file_, buf, len); }

        // return the number of uncompressed bytes actually written
        //! 将指定的数据写入文件，并返回实际写入的长度
        int write(StringPiece buf) { return ::gzwrite(file_, buf.data(), buf.size()); }

        // number of uncompressed bytes
        //! 返回当前文件的读写位置
        //! off_t类型用于指示文件的偏移量，常就是long类型
        off_t tell() const { return ::gztell(file_); }

#if ZLIB_VERNUM >= 0x1240
        // number of compressed bytes
  off_t offset() const { return ::gzoffset(file_); }
#endif

        // int flush(int f) { return ::gzflush(file_, f); }
        //! 只读
        static GzipFile openForRead(StringArg filename)
        {
            return GzipFile(::gzopen(filename.c_str(), "rbe"));
        }
        //! 追加
        static GzipFile openForAppend(StringArg filename)
        {
            return GzipFile(::gzopen(filename.c_str(), "abe"));
        }

        static GzipFile openForWriteExclusive(StringArg filename)
        {
            return GzipFile(::gzopen(filename.c_str(), "wbxe"));
        }

        static GzipFile openForWriteTruncate(StringArg filename)
        {
            return GzipFile(::gzopen(filename.c_str(), "wbe"));
        }
private:
    explicit GzipFile(gzFile file)
            : file_(file)
    {
    }
private:
    gzFile file_;

};
} // namespace muduo

#endif //LIBZV_GZIP_FILE_H
