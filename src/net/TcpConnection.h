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
    TcpConnection(EventLoop* loop, const std::string& name, int sockfd,
                  const InetAddr& localAddr, const InetAddr& peerAddr);
    ~TcpConnection();

    EventLoop* getLoop() const { return loop_; }
    const std::string& getName() const { return name_; }
    const InetAddr& getLocalAddr() const { return localAddr_; }
    const InetAddr& getPeerAddr() const { return peerAddr_; }

    bool isConnected() const { return state_ == kConnected; }
    bool isDisconnected() const { return state_ == kDisconnected; }

    void send(const void* message, int len);
    void send(const std::string& message);
    void send(Buffer* message);
    void shutdown();
    void forceClose();
    void forceCloseWithDelay(double seconds);
    void setTcpNoDelay(bool on);





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
