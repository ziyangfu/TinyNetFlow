//
// Created by fzy on 23-5-17.
//

#ifndef TINYNETFLOW_TCPCONNECTION_H
#define TINYNETFLOW_TCPCONNECTION_H
#include "InetAddr.h"
#include "Callbacks.h"
#include "Buffer.h"
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
    void send(Buffer* buf);
    void shutdown();
    void forceClose();
    void forceCloseWithDelay(double seconds);
    void setTcpNoDelay(bool on);

    void startRead();
    void stopRead();
    bool isReading() const { return reading; }

    void setContext(const std::any& context) { context_ = context; }
    const std::any& getContext() const { return context_; }
    std::any* getMutableContext() { return &context_; }

    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback& cb) { writeCompleteCallback_ = cb; }
    void setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark) {
        highWaterMarkCallback_ = cb;
        highWaterMark_ = highWaterMark;
    }

    Buffer* getInputBufferPtr() { return &inputBuffer_; }
    Buffer* getOutputBufferPtr() { return &outputBuffer_; }
    /*!
     * \private 内部使用 */
    void setCloseCallback(const CloseCallback& cb) { closeCallback_ = cb; }
    /*!
     * \brief 供 TcpServer 接收新连接时使用 */
    void connectEstablished();
    void connectDestroyed();

private:
    enum StateE { kDisconnected, kConnecting, kConnected, kDisconnecting };

    void handleRead(); // TODO: 时间戳
    void handleWrite();
    void handleClose();
    void handleError();

    void sendInLoop(const void* message, size_t len);
    void sendInLoop(const std::string& message);

    void shutdownInLoop();
    void forceCloseInLoop();
    void setState(StateE s)  { state_ = s; }

    const char* stateToString() const;
    void startReadInLoop();
    void stopReadInLoop();

private:

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
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    HighWaterMarkCallback highWaterMarkCallback_;
    CloseCallback closeCallback_;

    size_t highWaterMark_;
    Buffer inputBuffer_;
    Buffer outputBuffer_;
    std::any context_;

};

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
} // namespace netflow::net



#endif //TINYNETFLOW_TCPCONNECTION_H
