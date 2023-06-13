//
// Created by fzy on 23-5-15.
//

#include "SocketsOps.h"

#include <errno.h>
#include <sys/socket.h>
#include <sys/uio.h>  /** for readv */
#include <unistd.h>
#include <strings.h> /** bzero */
#include <string.h> /** memcmp */

using namespace netflow::net;

/** sockaddr_in to sockaddr */
/** TODO:
 * 1. 为什么要用 implicit_cast？
 * 2. implicit_cast 与 static_cast的区别是什么？
 * 3. 为什么要转换为 const void* */
const struct sockaddr* sockets::sockaddr_in_to_sockaddr(const struct sockaddr_in* addr){
    return static_cast<const struct sockaddr*>(static_cast<const void*>(addr));
}
/** sockaddr_in6 to sockaddr */
struct sockaddr* sockets::sockaddr_in6_to_sockaddr(struct sockaddr_in6* addr){
    return static_cast<struct sockaddr*>(static_cast<void*>(addr));
}

const struct sockaddr_in *sockets::sockaddr_to_sockaddr_in(const struct sockaddr *addr) {
    return static_cast<const struct sockaddr_in*>(static_cast<const void*>(addr));
}

const struct sockaddr_in6 *sockets::sockaddr_to_sockaddr_in6(const struct sockaddr *addr) {
    return static_cast<const struct sockaddr_in6*>(static_cast<const void*>(addr));
}


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
    int ret = ::listen(sockfd, SOMAXCONN);
    if(ret < 0){
        /** error */
    }
}
int sockets::accept(int sockfd, struct sockaddr_in6* addr){
    socklen_t addrlen = static_cast<socklen_t>(sizeof *addr);
    int connfd = ::accept4(sockfd, sockaddr_in6_to_sockaddr(addr), &addrlen,
                           SOCK_NONBLOCK | SOCK_CLOEXEC);
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
ssize_t sockets::write(int sockfd, const void* buf, size_t count){
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

struct sockaddr_in6 sockets::getLocalAddr(int sockfd) {
    struct sockaddr_in6 localaddr;
    bzero(&localaddr, sizeof localaddr);
    socklen_t addrlen = static_cast<socklen_t>(sizeof localaddr);
    if (::getsockname(sockfd, sockaddr_in6_to_sockaddr(&localaddr), &addrlen) < 0) {
        /** error */
    }
    return localaddr;
}

struct sockaddr_in6 sockets::getPeerAddr(int sockfd) {
    struct sockaddr_in6 peeraddr;
    bzero(&peeraddr, sizeof peeraddr);
    socklen_t addrlen = static_cast<socklen_t>(sizeof peeraddr);
    if (::getpeername(sockfd, sockaddr_in6_to_sockaddr(&peeraddr), &addrlen) < 0) {
        /** error */
    }
    return peeraddr;
}
/**
 * \brief 自连接即 源 IP，port 与 目标 IP port 完全一致
 * 出现场景：本地IPC通信
 * 表现形式：源 IP，port 与 目标 IP port 完全一致
 * 原因分析：
 *      1. 当socket 调用connect()时,os会为该socket分配一个被称作临时端口的源port作为bind,
 *          分配策略就是os维护一个计数,每次有进程申请就自增;
        2. 分配完临时端口后的socket,开始向目标端口发送SYN,因为都在本机,如果这个临时端口和目标端口恰好一致了,
        就会导致端口自己向自己发送SYN,从而触发两个active socket同时连接的处理逻辑,而os内部并没有对源ip:port
        和目的ip:port进行判断,所以TCP自连接发生了
  解决办法：
    1. 设置合适的临时端口分配段,保证不会和目标端口重叠
        vim  /etc/sysctl.conf
        net.ipv4.ip_local_port_range=1024  65535
    2. 在程序中判断（即本方法）
*/
bool sockets::isSelfConnect(int sockfd){
    struct sockaddr_in6 localaddr = getLocalAddr(sockfd);
    struct sockaddr_in6 peeraddr = getPeerAddr(sockfd);
    if(localaddr.sin6_family == AF_INET) {
        /** reinterpret_cast 运算符并不会改变括号中运算对象的值，而是对该对象从位模式上进行重新解释 */
        const sockaddr_in* laddr4 = reinterpret_cast<struct sockaddr_in*>(&localaddr);
        const sockaddr_in* paddr4 = reinterpret_cast<struct sockaddr_in*>(&peeraddr);
        return laddr4->sin_port == paddr4->sin_port && laddr4->sin_addr.s_addr == paddr4->sin_addr.s_addr;
    }
    else if(localaddr.sin6_family == AF_INET6) {
        return localaddr.sin6_port == peeraddr.sin6_port &&
                (memcmp(&localaddr.sin6_addr, &peeraddr.sin6_addr, sizeof localaddr.sin6_addr) == 0);
    }
    else {
        return false;
    }
}

int sockets::getSocketError(int sockfd) {
    int optval;
    socklen_t optlen = static_cast<socklen_t>(sizeof optval);
    if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
        return errno;
    }
    else {
        return optval;
    }
}

