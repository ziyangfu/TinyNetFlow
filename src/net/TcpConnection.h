//
// Created by fzy on 23-5-17.
//

#ifndef TINYNETFLOW_TCPCONNECTION_H
#define TINYNETFLOW_TCPCONNECTION_H
#include "InetAddr.h"
#include "Callbacks.h"

// Callback, Buffer
#include <memory>
#include <any>
#include <string>
#include <atomic>
namespace netflow::net {

class Channel;
class EventLoop;
class Socket;
/** TCP 连接处理， 供 TCP客户端与服务端用 */
class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
public:
    TcpConnection();
    ~TcpConnection();


private:
    enum StateE { kDisconnected, kConnecting, kConnected, kDisconnecting };
    EventLoop* loop_;
    const std::string name_;
    std::atomic<StateE> state_;
    bool reading;
    /** 以下类不暴露给客户端 */
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;
    const InetAddr localAddr_;
    const InetAddr peerAddr_;
    ConnectionCallback connectionCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    HighWaterMarkCallback highWaterMarkCallback_;
    CloseCallback closeCallback_;





};
} // namespace netflow::net



#endif //TINYNETFLOW_TCPCONNECTION_H
