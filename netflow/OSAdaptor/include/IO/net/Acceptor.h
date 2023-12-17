//
// Created by fzy on 23-5-16.
//

#ifndef TINYNETFLOW_ACCEPTOR_H
#define TINYNETFLOW_ACCEPTOR_H

#include "netflow/OSAdaptor/include/IO/reactor/Channel.h"
#include "Socket.h"
namespace netflow::net {

class EventLoop;
class InetAddr;
/** TCP 服务端 接收器 */
class Acceptor {
public:
    using NewConnectionCallback = std::function<void (int sockfd, const InetAddr&)>;
    Acceptor(EventLoop* loop, const InetAddr& listenAddr, bool reuseport);
    ~Acceptor();
    void setNewConnectionCallback(const NewConnectionCallback& cb)
    { newConnectionCallback_ = cb; }

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
} // namespace netflow::net

#endif //TINYNETFLOW_ACCEPTOR_H
