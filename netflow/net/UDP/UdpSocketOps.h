//
// Created by fzy on 23-8-24.
//

#ifndef TINYNETFLOW_UDPSOCKETOPS_H
#define TINYNETFLOW_UDPSOCKETOPS_H

#include <arpa/inet.h>
#include <sys/socket.h>
#include <string>

namespace netflow::net::udpSockets {

int createUdpSocketV4();
int createUdpSocketV6();

void bind();
bool createAndBind();
void setMulticastAddr();

void setUdpReuseAddr(int sockfd, bool on);
void setUdpReusePort(int sockfd, bool on);

/** IPv4 多播组 */
/** 在指定接口上加入组播组 IP_ADD_MEMBERSHIP */
bool joinMulticastGroupV4(int sockfd, const std::string& ip);
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
