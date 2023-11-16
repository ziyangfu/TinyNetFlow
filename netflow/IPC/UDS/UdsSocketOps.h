//
// Created by fzy on 23-11-16.
//

#ifndef TINYNETFLOW_UDSSOCKETOPS_H
#define TINYNETFLOW_UDSSOCKETOPS_H

#include <sys/un.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>



namespace netflow::net::udsSockets {

struct SocketPath {
    int domain;
    int path;
};

int createUdsSocket();
int connect(int fd, const struct sockaddr* addr);
void bind(int fd, const std::string& path);
void listen(int sockfd);
int accept(int sockfd, struct sockaddr_in6* addr);
ssize_t read(int fd, void* buf, size_t count);
ssize_t write(int fd, const void* buf, size_t count);
int close(int sockfd);
} // namespace netflow::net::udsSockets

#endif //TINYNETFLOW_UDSSOCKETOPS_H
