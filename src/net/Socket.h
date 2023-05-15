//
// Created by fzy on 23-5-11.
//

#ifndef TINYNETFLOW_SOCKET_H
#define TINYNETFLOW_SOCKET_H

#include <arpa/inet.h>

class Socket {
public:
    explicit Socket(int sockfd)
        :sockfd_(sockfd)
    {
    }
    ~Socket();

    int getFd() const { return sockfd_; }

    void bindAddr();
    void listen();
    int accept();

    void shutdownWrite();

    /** set socket option */
    void setTcpNoDelay(bool on);
    void setReuseAddr(bool on);
    void setReusePort(bool on);
    void setKeepAlive(bool on);
    /** for debug */
    bool getTcpInfo() const;
private:
    const int sockfd_;


};

} // namespace netflow::net



#endif //TINYNETFLOW_SOCKET_H
