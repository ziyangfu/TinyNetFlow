//
// Created by fzy on 23-3-13.
//

/*! 接收器，用于服务端接收连接 */
#ifndef LIBZV_ACCEPTOR_H
#define LIBZV_ACCEPTOR_H

#include <functional>
#include "channel.h"
#include "socket.h"

namespace muduo::net {

class EventLoop;
class InetAddress;

class Acceptor : Noncopyable {
public:
    using NewConnectionCallback = std::function<void (int sockfd, const InetAddress)>;
    Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport);
    ~Acceptor();

    void setNewConnectionCallback(const NewConnectionCallback cb) { newConnectionCallback_ = cb; }

    void listen();

    bool listening() const { return listening_; }
private:
    void handleRead();
private:
    EventLoop* loop_;
    Socket acceptSocket_;
    Channel acceptChannel_;
    NewConnectionCallback newConnectionCallback_;
    bool listening_;
    int idleFd_;

};
}  // namespace muduo::net


#endif //LIBZV_ACCEPTOR_H
