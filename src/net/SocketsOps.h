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
int accept(int sockfd, struct sockaddr_in* addr);
ssize_t read(int sockfd, void* buf, size_t count);
/** 允许单个系统调用读入或写出自一个或多个缓冲区 */
ssize_t readv(int sockfd, const struct iovec* iov, int iovcnt);
ssize_t write(int sockfd, void* buf, size_t count);
int close(int sockfd);
/** 单独关闭写功能 */
void shutdownWrite(int sockfd);

bool isSelfconnect(int sockfd);

/** sockaddr, sockaddr_in, sockaddr_in6 的转换函数 */
} // namespace netflow::net::sockets

#endif //TINYNETFLOW_SOCKETSOPS_H
