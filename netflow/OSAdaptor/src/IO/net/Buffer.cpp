//
// Created by fzy on 23-5-17.
//

#include "Buffer.h"

#include "SocketsOps.h"
#include <errno.h>
#include <sys/uio.h>

using namespace netflow::net;

/*!
 * \brief inputBuffer:从socket缓冲区读取数据到 inputBuffer
 * \arg
 *      fd: socket fd
 *      savedError: 存放错误代码
 * \return n:读取了多少数据到inputBuffer
 * \details 使用readv结合栈上空间解决了缓冲区自动伸缩问题
 * */
ssize_t Buffer::readFd(int fd, int *savedErrno) {
    char extrabuf[65536];
    struct iovec vec[2];
    const size_t writable = writableBytes();
    vec[0].iov_base = begin() + writerIndex_;
    vec[0].iov_len = writable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof extrabuf;

    const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;
    /** 从fd中读取数据，按照顺序写入vec，第一个vec写完，再写第二个vec */
    const ssize_t n = tcpSocket::readv(fd,vec, iovcnt);
    if (n < 0){
        *savedErrno = errno;
    }
    /** 数据都读到了buffer_里，extrabuf中没有数据 */
    else if (static_cast<size_t>(n) <= writable) {
        writerIndex_ += n;
    }
    /** buffer_写满了，并启用了 extrabuf */
    else {
        writerIndex_ = buffer_.size();
        /** 将 extrabuf中存入的有效数据，追加到buffer_后面， 此时将触发buffer_扩容 */
        append(extrabuf, n - writable);
    }
    return n;
}