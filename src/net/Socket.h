//
// Created by fzy on 23-5-11.
//

#ifndef TINYNETFLOW_SOCKET_H
#define TINYNETFLOW_SOCKET_H

#include <arpa/inet.h>

/** Linux POSIX socket API 封装 */
namespace netflow::net {

class Socket {
public:
    explicit Socket(int sockfd)
        :sockfd_(sockfd)
    {
    }
    ~Socket();
    /** socket API */
    int createNonblockingSocket(sa_family_t family);
    void connect(int sockfd, const struct sockaddr* addr);
    void bind(int sockfd, const struct sockaddr* addr);
    void listen(int sockfd);
    int accept(int sockfd, const struct sockaddr* addr);
    /** set socket option */
    void setTcpNoDelay(bool on);
    void setReuseAddr(bool on);
    void setReusePort(bool on);
    void setKeepAlive(bool on);
private:
    const int sockfd_;


};

} // namespace netflow::net



#endif //TINYNETFLOW_SOCKET_H
