//
// Created by fzy on 23-3-30.
//

#include "file_util.h"
#include "logging.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>   //! 文件控制相关的头文件，包括打开和关闭文件、锁定和解锁文件等。
#include <stdio.h>  //! 标准输入输出库的头文件，定义了文件和流相关的操作函数，如 fopen()、fclose()、fread()、fwrite() 等。
#include <sys/stat.h> //! 文件状态的头文件，定义了 stat() 函数用于获取文件状态信息，如文件大小、修改时间等。
#include <unistd.h>  //! POSIX 标准定义的头文件，包括了各种 Unix 系统调用，如 read()、write()、close() 等

using namespace muduo;


FileUtil::AppendFile::AppendFile(StringArg filename)
        : fp_(::fopen(filename.c_str(), "ae")),  // 'e' for O_CLOEXEC
          writtenBytes_(0)
{
    assert(fp_);
    ::setbuffer(fp_, buffer_, sizeof buffer_);
    // posix_fadvise POSIX_FADV_DONTNEED ?
}

FileUtil::AppendFile::~AppendFile()
{
    ::fclose(fp_);
}

void FileUtil::AppendFile::append(const char* logline, const size_t len)
{
    size_t written = 0;

    while (written != len)
    {
        size_t remain = len - written;
        size_t n = write(logline + written, remain);
        if (n != remain)
        {
            int err = ferror(fp_);
            if (err)
            {
                fprintf(stderr, "AppendFile::append() failed %s\n", strerror_tl(err));
                break;
            }
        }
        written += n;
    }

    writtenBytes_ += written;
}

void FileUtil::AppendFile::flush()
{
    ::fflush(fp_);
}

size_t FileUtil::AppendFile::write(const char* logline, size_t len)
{
    // #undef fwrite_unlocked
    return ::fwrite_unlocked(logline, 1, len, fp_);
}

FileUtil::ReadSmallFile::ReadSmallFile(StringArg filename)
        : fd_(::open(filename.c_str(), O_RDONLY | O_CLOEXEC)),
          err_(0)
{
    buf_[0] = '\0';
    if (fd_ < 0)
    {
        err_ = errno;
    }
}

FileUtil::ReadSmallFile::~ReadSmallFile()
{
    if (fd_ >= 0)
    {
        ::close(fd_); // FIXME: check EINTR
    }
}

// return errno
template<typename String>
int FileUtil::ReadSmallFile::readToString(int maxSize,
                                          String* content,
                                          int64_t* fileSize,
                                          int64_t* modifyTime,
                                          int64_t* createTime)
{
    static_assert(sizeof(off_t) == 8, "_FILE_OFFSET_BITS = 64");
    assert(content != NULL);
    int err = err_;
    if (fd_ >= 0)
    {
        content->clear();

        if (fileSize)
        {
            struct stat statbuf;
            if (::fstat(fd_, &statbuf) == 0)
            {
                if (S_ISREG(statbuf.st_mode))
                {
                    *fileSize = statbuf.st_size;
                    content->reserve(static_cast<int>(std::min(implicit_cast<int64_t>(maxSize), *fileSize)));
                }
                else if (S_ISDIR(statbuf.st_mode))
                {
                    err = EISDIR;
                }
                if (modifyTime)
                {
                    *modifyTime = statbuf.st_mtime;
                }
                if (createTime)
                {
                    *createTime = statbuf.st_ctime;
                }
            }
            else
            {
                err = errno;
            }
        }

        while (content->size() < implicit_cast<size_t>(maxSize))
        {
            size_t toRead = std::min(implicit_cast<size_t>(maxSize) - content->size(), sizeof(buf_));
            ssize_t n = ::read(fd_, buf_, toRead);
            if (n > 0)
            {
                content->append(buf_, n);
            }
            else
            {
                if (n < 0)
                {
                    err = errno;
                }
                break;
            }
        }
    }
    return err;
}

int FileUtil::ReadSmallFile::readToBuffer(int* size)
{
    int err = err_;
    if (fd_ >= 0)
    {
        ssize_t n = ::pread(fd_, buf_, sizeof(buf_)-1, 0);
        if (n >= 0)
        {
            if (size)
            {
                *size = static_cast<int>(n);
            }
            buf_[n] = '\0';
        }
        else
        {
            err = errno;
        }
    }
    return err;
}

template int FileUtil::readFile(StringArg filename,
                                int maxSize,
                                string* content,
                                int64_t*, int64_t*, int64_t*);

template int FileUtil::ReadSmallFile::readToString(
        int maxSize,
        string* content,
        int64_t*, int64_t*, int64_t*);



