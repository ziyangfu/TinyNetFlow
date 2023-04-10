//
// Created by fzy on 23-3-13.
//

/*!
 * 在C语言中进行网络编程时，常用的头文件通常包括以下几个：

<sys/types.h>：该头文件定义了系统调用函数中使用的一些基本类型，例如 size_t、ssize_t、off_t 等。

<sys/socket.h>：该头文件定义了套接字(Socket)编程中使用的函数、数据结构和常量，包括创建、配置、连接、发送、接收、关闭套接字等操作的函数，
 以及套接字的类型、协议、地址等的定义。

<netdb.h>：该头文件定义了用于访问和解析网络主机信息的函数和数据结构，包括获取主机名和IP地址之间的转换、获取服务名和端口号之间的转换、
 获取本机主机信息等。

<netinet/in.h>：该头文件定义了用于处理网络协议的数据结构和常量，包括 IPv4 和 IPv6 地址的定义、套接字地址结构的定义、网络字节序
 和主机字节序之间的转换函数、IP协议头的定义等。

<arpa/inet.h>：该头文件定义了用于进行 IP 地址和二进制数值之间转换的函数，例如 inet_addr()、inet_ntoa()、inet_pton()、
 inet_ntop() 等。

<unistd.h>：该头文件定义了一些系统调用函数，例如 close()、read()、write() 等，用于对文件描述符进行操作，包括套接字。

这些头文件通常在进行网络编程时一起使用，以便使用其中定义的函数和数据结构来进行网络通信、地址解析、协议处理等操作。需要注意的是，
 不同的操作系统和编译器可能对这些头文件的具体实现和支持的特性有所不同，因此在编写网络编程代码时需要参考相应的文档和规范，并遵循
 网络编程的规范和标准。*/


#ifndef LIBZV_SOCKET_H
#define LIBZV_SOCKET_H

#include "../base/noncopyable.h"

struct tcp_info;

namespace muduo {
namespace net {

class InetAddress;

/*!
 * 封装 socket 文件描述符
 * 析构时自动关闭 sockfd
 * 线程安全 */
 //! 不写public即默认私有继承
 class Socket : Noncopyable {
 public:
     explicit Socket(int sockfd)
        : sockfd_(sockfd)
     {}
     ~Socket();

     int fd() const { return sockfd_; }

     bool getTcpInfo(struct tcp_info* tcpi) const;
     bool getTcpInfoString(char* buf, int len) const;

     void bindAddress(const InetAddress& localaddr);

     void listen();

     int accept(InetAddress* peeraddr);

     void shutdownWrite();

     void setTcpNoDelay(bool on);

     void setReuseAddr(bool on);

     void setReusePort(bool on);

     void setKeepAlive(bool on);


 private:
     const int sockfd_;
 };
} // namespace net
} // namespace muduo
#endif //LIBZV_SOCKET_H
