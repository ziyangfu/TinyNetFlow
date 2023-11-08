//
// Created by fzy on 23-8-24.
//

#ifndef TINYNETFLOW_UDPSOCKETOPS_H
#define TINYNETFLOW_UDPSOCKETOPS_H

#include <arpa/inet.h>
#include <sys/socket.h>
#include <string>
namespace netflow::net::udpSockets {

int createUdpSocket(sa_family_t family);
/** 绑定本地地址， 一般 UDP server 用
 *  1. 在UDP编程中，bind函数用于将一个特定的IP地址和端口号绑定到UDP套接字上。
    2. 绑定操作是可选的，如果不进行绑定，系统会自动分配一个临时的IP地址和端口号。
    3. 绑定后，UDP套接字就可以通过该IP地址和端口号来接收数据。
*/
void bind(int fd, const struct sockaddr* addr);
int createAndBindSocket(const struct sockaddr* addr, bool isUseV6);
/** 保存远端地址，一般 UDP client 用
 *  1. 在UDP编程中，connect函数也可以使用，但它的作用不同于TCP。
    2. 在UDP中，connect函数并不会建立一个真正的连接，而是将UDP套接字与指定的目标IP地址和端口号进行关联。
    3. 通过connect函数关联后，就可以使用send和recv函数来发送和接收数据，而不需要每次都指定目标地址。
    4. connect函数可以用于简化代码，并提供一种类似于TCP连接的数据发送和接收方式
*/
int connect(int fd, const struct sockaddr* addr);
bool sendTo(int fd, const struct sockaddr* addr, const char* data, size_t length);
bool sendTo(int fd, const struct sockaddr* addr, const std::string& message);
int recvFrom(int fd, void* buf, size_t length, sockaddr* addr);
bool send(int fd, const char* data, size_t length);
ssize_t read(int fd, void* buf, size_t count);
ssize_t write(int fd, void* buf, size_t count);
int close(int fd);


void setMulticastAddr();

void setUdpReuseAddr(int sockfd, bool on);
void setUdpReusePort(int sockfd, bool on);

/** IPv4 多播组 */
/** 在指定接口上加入组播组 IP_ADD_MEMBERSHIP */
bool joinMulticastGroupV4(int sockfd, const struct sockaddr_in* addr);
/** 退出组播组 IP_DROP_MEMBERSHIP */
bool leaveMulticastGroupV4(int sockfd, const std::string& ip);
/** 设置多播组数据的TTL值, IP_MULTICAST_TTL */
void setMulticastTtlV4(int sockfd, int ttl);
/** 获取默认接口或设置接口 IP_MULTICAST_IF */
void setMulticastNetworkInterfaceV4(int sockfd, const std::string& ip);
/** 禁止组播数据回送 IP_MULTICAST_LOOP */
void setMulticastLoopV4(int sockfd, bool on);

/** IPv6 多播组 */
bool joinMulticastGroupV6(int sockfd, const std::string& ip6);
bool leaveMulticastGroupV6(int sockfd, const std::string& ip6);
void setMulticastTtlV6(int sockfd, int ttl);
void setMulticastNetworkInterfaceV6(int sockfd, const sockaddr_in6* addr6);
void setMulticastLoopV6(int sockfd, bool on);

}  // namespace netflow::net::udpSockets


#endif //TINYNETFLOW_UDPSOCKETOPS_H
