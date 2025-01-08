//
// Created by fzy on 2025/1/7.
//

#ifndef TINYNETFLOW_ACCEPTOR_H
#define TINYNETFLOW_ACCEPTOR_H

#include "IO/reactor/Channel.h"
#include "IO/net/TcpServerSocket.h"

namespace osadaptor::net {

class EventLoop;
class InetAddr;

class Acceptor {
public:
    using NewConnectionCallback = std::function<void (int sockfd, const InetAddr&)>;
private:
    EventLoop* loop_;
    TcpServerSocket acceptSocket_;
    Channel acceptChannel_;
    bool listening_;

    NewConnectionCallback newConnectionCallback_;
public:
    /** loop 不一定需要，目前只用来确认，acceptor的所有函数是在loop线程上运行 */
    Acceptor(EventLoop* loop, const InetAddr& listenAddr, bool reuseport);

    ~Acceptor();
    void setNewConnectionCallback(const NewConnectionCallback& cb)
    { newConnectionCallback_ = cb; }

    void listen();

    bool listening() const { return listening_; }

private:
    void handleRead();
};

#endif //TINYNETFLOW_ACCEPTOR_H
