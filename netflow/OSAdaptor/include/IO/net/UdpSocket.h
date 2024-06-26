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
 *      UDP socket接口
 * \file
 *      UdpSocket.h
 * ----------------------------------------------------------------------------------------- */

#ifndef TINYNETFLOW_OSADAPTOR_UDPSOCKET_H
#define TINYNETFLOW_OSADAPTOR_UDPSOCKET_H

#include <arpa/inet.h>
#include <sys/socket.h>
#include <string>

namespace osadaptor::net::udpSocket {

/** ------------------------ UDP 部分 ------------------------------------------------------ */
int createNoBlockingUdpSocket(sa_family_t family);
int createBlockingUdpSocket(sa_family_t family);
void setBlockingMode(int fd, bool noblock);

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
void bind(int fd, const struct sockaddr* addr);

/*!
 * \details
 * 保存远端地址，一般 UDP client 用
 *  1. 在UDP编程中，connect函数也可以使用，但它的作用不同于TCP。
    2. 在UDP中，connect函数并不会建立一个真正的连接，而是将UDP套接字与指定的目标IP地址和端口号进行关联。
    3. 通过connect函数关联后，就可以使用send和recv函数来发送和接收数据，而不需要每次都指定目标地址。
    4. connect函数可以用于简化代码，并提供一种类似于TCP连接的数据发送和接收方式

    如果UDP server使用connect，将导致服务器只接受这特定一个主机的请求
*/
int connect(int fd, const struct sockaddr* addr);
ssize_t sendTo(int fd, const struct sockaddr* addr, const void* data, size_t length);
ssize_t recvFrom(int fd, char* buf, size_t length, sockaddr* addr);

void send(int fd, const char* data, size_t length);
ssize_t read(int fd, void* buf, size_t count);
ssize_t write(int fd, const void* buf, size_t count);
int close(int sockfd);

void setUdpReuseAddr(int sockfd, bool on);
void setUdpReusePort(int sockfd, bool on);
void setReceiveBufferSize(int sockfd, int size);
void setSendBufferSize(int sockfd, int size);
void setPriority(int sockfd, int32_t priority);
void setUnicastTtl(int sockfd, uint8_t hops);

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

/**
函数：setsockopt
描述：配置发送端UDP组播属性
参数：
	sockfd：	用于 UDP 通信的套接字
	level：		套接字级别，设置组播属性需要将该参数指定为：IPPTOTO_IP
	optname: 	套接字选项名，设置组播属性需要将该参数指定为：IP_MULTICAST_IF
	optval：	设置组播属性，这个指针需要指向一个 struct in_addr{} 类型的结构体地址，
                这个结构体地址用于存储组播地址，并且组播 IP 地址的存储方式是大端的（网络字节序）。
	optlen：optval 指针指向的内存大小，即：sizeof(struct in_addr)

返回值：函数调用成功返回 0，调用失败返回 - 1
*/

}  // namespace osadaptor::net::udpSocket




#endif //TINYNETFLOW_OSADAPTOR_UDPSOCKET_H
