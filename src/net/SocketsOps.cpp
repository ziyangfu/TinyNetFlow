//
// Created by fzy on 23-5-15.
//

#include "SocketsOps.h"

#include <errno.h>
#include <sys/socket.h>
#include <sys/uio.h>  /** for readv */
#include <unistd.h>

using namespace netflow::net;

int sockets::createNonblockingSocket(sa_family_t family){
    int sockfd = ::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if(sockfd < 0) {
        /** error */
    }
    return sockfd;
}
int sockets::connect(int sockfd, const struct sockaddr* addr){
    int ret = ::connect(sockfd, addr, sizeof(struct sockaddr));
    return ret;
}
void sockets::bind(int sockfd, const struct sockaddr* addr){
    int ret = ::bind(sockfd, addr, sizeof(struct sockaddr));
    if(ret < 0){
        /** error */
    }
}
void sockets::listen(int sockfd){
    int ret = ::listen(sockfd);
    if(ret < 0){
        /** error */
    }
}
int sockets::accept(int sockfd, struct sockaddr_in* addr){
    socklen_t addrlen = static_cast<socklen_t>(sizeof *addr);
    const struct sockaddr* skaddr = static_cast<const struct sockaddr*>(addr);
    int connfd = ::accept4(sockfd, skaddr, &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if(connfd < 0){
        int savedErrno = errno;
        switch (savedErrno)
        {
            case EAGAIN:
            case ECONNABORTED:
            case EINTR:
            case EPROTO: // ???
            case EPERM:
            case EMFILE: // per-process lmit of open file desctiptor ???
                // expected errors
                errno = savedErrno;
                break;
            case EBADF:
            case EFAULT:
            case EINVAL:
            case ENFILE:
            case ENOBUFS:
            case ENOMEM:
            case ENOTSOCK:
            case EOPNOTSUPP:
                // unexpected errors
                //LOG_FATAL << "unexpected error of ::accept " << savedErrno;
                break;
            default:
                //LOG_FATAL << "unknown error of ::accept " << savedErrno;
                break;
        }
    }
    return connfd;
}
ssize_t sockets::read(int sockfd, void* buf, size_t count){
    return ::read(sockfd, buf, count);

}
/** 允许单个系统调用读入或写出自一个或多个缓冲区 */
ssize_t sockets::readv(int sockfd, const struct iovec* iov, int iovcnt){
    return ::readv(sockfd, iov, iovcnt);

}
ssize_t sockets::write(int sockfd, void* buf, size_t count){
    return ::write(sockfd, buf, count);

}
int sockets::close(int sockfd){
    return ::close(sockfd);
}
/** 单独关闭写功能
 * ::shutdown :
           Shut down all or part of the connection open on socket FD.
           HOW determines what to shut down:
             SHUT_RD   = No more receptions;
             SHUT_WR   = No more transmissions;
             SHUT_RDWR = No more receptions or transmissions.
           Returns 0 on success, -1 for errors.
   */
void sockets::shutdownWrite(int sockfd){
    ::shutdown(sockfd, SHUT_WR);
}

bool sockets::isSelfconnect(int sockfd){
    // PASS
}