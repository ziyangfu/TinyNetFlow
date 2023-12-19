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
 *      socket接口
 * \file
 *      SocketInterface.h
 * ----------------------------------------------------------------------------------------- */

#ifndef TINYNETFLOW_OSADAPTOR_SOCKETINTERFACE_H
#define TINYNETFLOW_OSADAPTOR_SOCKETINTERFACE_H

#include <cstdlib>
#include <cstdint>
#include <arpa/inet.h>

namespace netflow::osadaptor::net::tcpSocket {
int createSocket(std::int32_t family, std::int32_t type, std::int32_t protocol) noexcept;
int createNonblockingSocket(sa_family_t family) noexcept;

void close(int fd) noexcept;

void bind();

void setNoBlocking();

void getSockName();

void getPeerSocket();

void setSocketOption(/** fd, level, optionName ...*/);

void getSocketOption();




int connect(int sockfd, const struct sockaddr* addr);
void bind(int sockfd, const struct sockaddr* addr);
void listen(int sockfd);
int accept(int sockfd, struct sockaddr_in6* addr);
ssize_t read(int sockfd, void* buf, size_t count);
/** 允许单个系统调用读入或写出自一个或多个缓冲区 */
ssize_t readv(int sockfd, const struct iovec* iov, int iovcnt);
ssize_t write(int sockfd, const void* buf, size_t count);
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






// -------------------------------------------------------------------------------------
// uds



} // namespace netflow::osadaptor::net::tcpSocket


#endif //TINYNETFLOW_OSADAPTOR_SOCKETINTERFACE_H
