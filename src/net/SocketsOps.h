//
// Created by fzy on 23-5-15.
//

#ifndef TINYNETFLOW_SOCKETSOPS_H
#define TINYNETFLOW_SOCKETSOPS_H

/** POSIX socket API 封装 */
#include <arpa/inet.h>

namespace netflow::net::sockets {

int createNonblockingSocket(sa_family_t family);
int connect(int sockfd, const struct sockaddr* addr);
void bind(int sockfd, const struct sockaddr* addr);
void listen(int sockfd);
int accept(int sockfd, struct sockaddr* addr);
ssize_t read(int sockfd, void* buf, size_t count);
/** 允许单个系统调用读入或写出自一个或多个缓冲区 */
ssize_t readv(int sockfd, const struct iovec* iov, int iovcnt);
ssize_t write(int sockfd, void* buf, size_t count);
int close(int sockfd);
/** 单独关闭写功能 */
void shutdownWrite(int sockfd);

bool isSelfconnect(int sockfd);
struct sockaddr_in6 getLocalAddr(int sockfd);
struct sockaddr_in6 getPeerAddr(int sockfd);

int getSocketError(int sockfd);
/** sockaddr, sockaddr_in, sockaddr_in6 的转换函数 */
/** sockaddr_in to sockaddr */
const struct sockaddr* sockaddr_in_to_sockaddr(const struct sockaddr_in* addr);
/** sockaddr_in6 to sockaddr */
const struct sockaddr* sockaddr_in6_to_sockaddr(const struct sockaddr_in6* addr);
struct sockaddr* sockaddr_in6_to_sockaddr(struct sockaddr_in6* addr);
/** sockaddr to sockaddr_in */
const struct sockaddr_in* sockaddr_to_sockaddr_in(const struct sockaddr* addr);
/** sockaddr to sockaddr_in6 */
const struct sockaddr_in6* sockaddr_to_sockaddr_in6(const struct sockaddr* addr);
} // namespace netflow::net::sockets

#endif //TINYNETFLOW_SOCKETSOPS_H
