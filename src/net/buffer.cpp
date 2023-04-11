//
// Created by fzy on 23-3-13.
//

#include "buffer.h"
#include "socket_ops.h"
#include <errno.h>
#include <sys/uio.h>

using namespace muduo;
using namespace muduo::net;

const char Buffer::KCRLF[] = "\r\n";
const size_t Buffer::kCheapPrepend;
const size_t Buffer::kInitialSize;

ssize_t Buffer::readFd(int fd, int *savedErrno) {

}
