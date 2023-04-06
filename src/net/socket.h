//
// Created by fzy on 23-3-13.
//


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

     bool getTcpInfo(struct tcp_info*) const;
     bool getTcpInfoString(char* buf, int len) const;

     void bindAddress(const InetAddress& localaddr);

     void listen();

     void accept(InetAddress* peeraddr);

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
