/** ----------------------------------------------------------------------------------------
 * \copyright
 * Copyright (c) 2023 by the TinyNetFlow project authors. All Rights Reserved.
 *
 * This file is open source software, licensed to you under the ter；ms
 * of the Apache License, Version 2.0 (the "License").  See the NOTICE file
 * distributed with this work for additional information regarding copyright
 * ownership.  You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 * -----------------------------------------------------------------------------------------
 * \brief
 *      TCP socket接口封装，POSIX socket API
 * \file
 *      TcpSocket.h
 * ----------------------------------------------------------------------------------------- */

#ifndef TINYNETFLOW_OSADAPTOR_TCPSOCKET_H
#define TINYNETFLOW_OSADAPTOR_TCPSOCKET_H

#include <arpa/inet.h>

namespace netflow::osadaptor::net::tcpSocket {

int createNonblockingSocket(sa_family_t family);
int createBlockingSocket(sa_family_t family);
void setBlockingMode(int sockfd, bool noblock);

int connect(int sockfd, const struct sockaddr* addr);
void bind(int sockfd, const struct sockaddr* addr);
void listen(int sockfd);
int accept(int sockfd, struct sockaddr_in6* addr);
ssize_t read(int sockfd, void* buf, size_t count);
/** 允许单个系统调用读入或写出自一个或多个缓冲区 */
ssize_t readv(int sockfd, const struct iovec* iov, int iovcnt);
ssize_t write(int sockfd, const void* buf, size_t count);
int close(int sockfd);
/** 单独关闭写功能 */
void shutdownWrite(int sockfd);

void toIpPort(char* buf, size_t size, const struct sockaddr* addr);
void toIp(char* buf, size_t size, const struct sockaddr* addr);

void fromIpPort(const char* ip, uint16_t port, struct sockaddr_in* addr);
void fromIpPort(const char* ip, uint16_t port, struct sockaddr_in6* addr);
bool isSelfConnect(int sockfd);
struct sockaddr_in6 getLocalAddr(int sockfd);
struct sockaddr_in6 getPeerAddr(int sockfd);

int getSocketError(int sockfd);

void setTcpNoDelay(int sockfd, bool on);
void setReuseAddr(int sockfd, bool on);
void setReusePort(int sockfd, bool on);
void setKeepAlive(int sockfd, bool on);
void setPriority(int sockfd, int32_t priority);
void setUnicastTtl(int sockfd, uint8_t hops);

} // namespace netflow::net::tcpSocket

#endif //TINYNETFLOW_OSADAPTOR_TCPSOCKET_H
