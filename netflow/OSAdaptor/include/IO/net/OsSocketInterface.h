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
 *      OS socket接口封装，POSIX socket API
 * \file
 *      OsSocketInterface.h
 * ----------------------------------------------------------------------------------------- */

#ifndef TINYNETFLOW_OSADAPTOR_OSSOCKETINTERFACE_H
#define TINYNETFLOW_OSADAPTOR_OSSOCKETINTERFACE_H

#include <arpa/inet.h>
#include "IO/net/SocketType.h"

namespace osadaptor {
namespace net {
namespace socketInterface {

/*!
 *  基本socket调用
 *  发送与接收
 *  TCP/UDP选项设置，包括组播
 *  字符串与地址的转换
 *  网络接口层相关配置
 *
 * */


/*!
 * \brief 创建非阻塞TCP socket
 * */
int createNonblockingSocket(sa_family_t family, int32_t type) noexcept;

int createBlockingSocket(SocketFamily family, SocketType type);
void setBlockingMode(int sockfd, bool noblock);
int close(int sockfd) noexcept;

void bind(int sockfd, const struct sockaddr* addr);
void listen(int sockfd, std::int32_t backlog);
int accept(int sockfd, struct sockaddr_in6* addr);
int connect(int sockfd, const struct sockaddr* addr);

void getSocketName(int fd);
void getPeerName(int fd);
int getSocketError(int sockfd);
struct sockaddr_in6 getLocalAddr(int sockfd);
struct sockaddr_in6 getPeerAddr(int sockfd);

/** 单独关闭写功能 */
void shutdownWrite(int sockfd);


ssize_t read(int sockfd, void* buf, size_t count);
/** 允许单个系统调用读入或写出自一个或多个缓冲区 */
ssize_t readv(int sockfd, const struct iovec* iov, int iovcnt);
ssize_t write(int sockfd, const void* buf, size_t count);

ssize_t sendTo(int fd, const struct sockaddr* addr, const void* data, size_t length);
ssize_t recvFrom(int fd, char* buf, size_t length, sockaddr* addr);

void sendDatagram(int fd, const char* data, size_t length);
void receiveDatagram(int fd, const char* data, size_t length);

void setReceiveBufferSize(int sockfd, int size);
void setSendBufferSize(int sockfd, int size);


void toIpPort(char* buf, size_t size, const struct sockaddr* addr);
void toIp(char* buf, size_t size, const struct sockaddr* addr);

void fromIpPort(const char* ip, uint16_t port, struct sockaddr_in* addr);
void fromIpPort(const char* ip, uint16_t port, struct sockaddr_in6* addr);
bool isSelfConnect(int sockfd);

/*! 通用option设置部分 */
/**
函数：setsockopt
描述：配置发送端UDP组播属性
参数：
	sockfd：	用于 UDP/TCP 通信的套接字
	level：		套接字级别，设置组播属性需要将该参数指定为：IPPTOTO_IP
	optname: 	套接字选项名，设置组播属性需要将该参数指定为：IP_MULTICAST_IF
	optval：	设置组播属性，这个指针需要指向一个 struct in_addr{} 类型的结构体地址，
                这个结构体地址用于存储组播地址，并且组播 IP 地址的存储方式是大端的（网络字节序）。
	optlen：optval 指针指向的内存大小，即：sizeof(struct in_addr)

返回值：函数调用成功返回 0，调用失败返回 - 1
*/
void setReuseAddr(int sockfd, bool on);
void setReusePort(int sockfd, bool on);
void setPriority(int sockfd, int32_t priority);
void setUnicastTtl(int sockfd, uint8_t hops);
/*! TCP option设置部分 */
void setTcpNoDelay(int sockfd, bool on);
void setKeepAlive(int sockfd, bool on);
/*! UDP option 设置部分 */
/** ------------------------ UDP 组播部分 ------------------------------------------------------ */
void setMulticastAddr();
/** IPv4 多播组 */
/** 在指定接口上加入组播组 IP_ADD_MEMBERSHIP */
bool joinMulticastGroupV4(int sockfd, const sockaddr_in* addr);
/** 退出组播组 IP_DROP_MEMBERSHIP */
bool leaveMulticastGroupV4(int sockfd, const sockaddr_in* addr);
/** 设置多播组数据的TTL值, IP_MULTICAST_TTL */
void setMulticastTtlV4(int sockfd, int ttl);
/** 获取默认接口或设置接口 IP_MULTICAST_IF */
void setMulticastNetworkInterfaceV4(int sockfd, const struct sockaddr_in* addr);
/** 允许或禁止发送组播数据 IP_MULTICAST_LOOP */
void setMulticastLoopV4(int sockfd, bool on);
/** IPv6 多播组 */
bool joinMulticastGroupV6(int sockfd, const sockaddr_in6* addr6);
bool leaveMulticastGroupV6(int sockfd,  const sockaddr_in6* addr6);
void setMulticastTtlV6(int sockfd, int ttl);
void setMulticastNetworkInterfaceV6(int sockfd, const sockaddr_in6* addr6);
void setMulticastLoopV6(int sockfd, bool on);

/** ------------------------ UDP 广播部分 ------------------------------------------------------ */
int setBroadcast(int fd, bool on);

/**  网络接口层部分,先放着 */
const char* networkIndexToName(std::int32_t index);
std::int32_t networkNameToIndex(const char* name);
void getNetInterfaceAddr();
void freeNetInterfaceAddr();
void findInterface();
void getName();



/*!
 * \details
 * 绑定本地地址， 一般 UDP server 用
 *  1. 在UDP编程中，bind函数用于将一个特定的IP地址和端口号绑定到UDP套接字上。
    2. 绑定操作是可选的，如果不进行绑定，系统会自动分配一个临时的IP地址和端口号。
    3. 绑定后，UDP套接字就可以通过该IP地址和端口号来接收数据。

    如果服务器程序就绪后一上来就要发送数据给客户端，那么服务器就需要知道客户端的地址信息和端口，
    那么就不能让客户端的地址信息和端口号由客户端所在操作系统分配，而是要在客户端程序指定了。
    此时就需要客户端程序 bind 指定地址
*/

/*!
 * \details
 * 保存远端地址，一般 UDP client 用
 *  1. 在UDP编程中，connect函数也可以使用，但它的作用不同于TCP。
    2. 在UDP中，connect函数并不会建立一个真正的连接，而是将UDP套接字与指定的目标IP地址和端口号进行关联。
    3. 通过connect函数关联后，就可以使用send和recv函数来发送和接收数据，而不需要每次都指定目标地址。
    4. connect函数可以用于简化代码，并提供一种类似于TCP连接的数据发送和接收方式

    如果UDP server使用connect，将导致服务器只接受这特定一个主机的请求
*/

}  // namespace socketInterface
}  // namespace net
}  // namespace osadaptor

#endif //TINYNETFLOW_OSADAPTOR_OSSOCKETINTERFACE_H
