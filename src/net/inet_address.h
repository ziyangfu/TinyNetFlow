//
// Created by fzy on 23-3-13.
//

/*! IP 地址封装 */
#ifndef LIBZV_INET_ADDRESS_H
#define LIBZV_INET_ADDRESS_H

#include "../base/copyable.h"
#include "../base/string_piece.h"

#include <netinet/in.h>

namespace muduo::net {

namespace sockets {

const struct sockaddr* sockaddr_cast(const struct sockaddr_in6* addr);

} // namespace sockets

/*! 封装 sockaddr_in */
class InetAddress : public muduo::Copyable {
public:
    explicit InetAddress(uint16_t port = 0,  bool loopbackOnly = false, bool ipv6 = false);
    InetAddress(StringArg ip, uint16_t port, bool ipv6 = false);

    explicit InetAddress(const struct sockaddr_in& addr)
            : addr_(addr) {}
    explicit InetAddress(const struct sockaddr_in6& addr6)
            : addr6_(addr6) {}

    sa_family_t family() const { return addr_.sin_family; }
    string toIp() const;
    string toIpPort() const;
    uint16_t port() const;

    const struct sockaddr* getSockAddr() const { return sockets::sockaddr_cast(&addr6_); }

    void setSockAddrInet6(const struct sockaddr_in6& addr6) { addr6_ = addr6; }

    uint32_t ipv4NetEndian() const;
    uint16_t portNetEndian() const { return addr_.sin_port; }

    static bool resolve(StringArg hostname, InetAddress* result);

    void setScopeId(uint32_t scope_id);

private:
    /*! union是一种特殊的数据结构，可以在同一内存空间中存储不同的数据类型，但同一时刻只能存储其中的一种类型。
     * union的大小取决于其成员中最大的数据类型，它的所有成员共享同一块内存空间
     * 作用：
     * 节省内存空间：由于union的所有成员共享同一块内存空间，所以在某一时刻只有一个成员是有效的，这可以节省内存空间。
     * 例如，在一个系统中需要同时存储整型、浮点型和字符型数据，但每次只需要其中一种类型的值，使用union可以避免为每
     * 种类型都分配独立的内存空间。*/
    union {
    struct sockaddr_in addr_;
    struct sockaddr_in6 addr6_;
    };

};
} // namespace muduo::net
#endif //LIBZV_INET_ADDRESS_H
