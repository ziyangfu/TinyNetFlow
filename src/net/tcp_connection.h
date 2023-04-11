//
// Created by fzy on 23-3-13.
//

#ifndef LIBZV_TCP_CONNECTION_H
#define LIBZV_TCP_CONNECTION_H

#include "../base/noncopyable.h"
#include "../base/string_piece.h"
#include "../base/types.h"

#include "callback.h"
#include "buffer.h"
#include "inet_address.h"

#include <memory>
#include <any>

/** <netinet/tcp.h> */
struct tcp_info;
namespace muduo::net {

class EventLoop;
class Channel;
class Socket;

class TcpConnection : Noncopyable,
                      public std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(EventLoop* loop, const string& name, int sockfd,
                  const InetAddress& localAddr, const InetAddress& peerAddr);
    ~TcpConnection();

    EventLoop* getLoop() const { return loop_; }
    const string& name() const { return name_; }
    const InetAddress& localAddress() const { return localAddr_; }
    const InetAddress& peerAddress() const { return peerAddr_; }
    bool connected() const { return state_ == kConnected; }
    bool disconnected() const { return state_ == kDisconnected; }
    // return true if success.
    bool getTcpInfo(struct tcp_info*) const;
    string getTcpInfoString() const;

    // void send(string&& message); // C++11
    void send(const void* message, int len);
    void send(const StringPiece& message);
    // void send(Buffer&& message); // C++11
    void send(Buffer* message);  // this one will swap data
    void shutdown(); // NOT thread safe, no simultaneous calling
    // void shutdownAndForceCloseAfter(double seconds); // NOT thread safe, no simultaneous calling
    void forceClose();
    void forceCloseWithDelay(double seconds);
    void setTcpNoDelay(bool on);
    // reading or not
    void startRead();
    void stopRead();
    bool isReading() const { return reading_; }; // NOT thread safe, may race with start/stopReadInLoop

    void setContext(const std::any& context)
    { context_ = context; }

    const std::any& getContext() const
    { return context_; }

    std::any* getMutableContext()
    { return &context_; }

    void setConnectionCallback(const ConnectionCallback& cb)
    { connectionCallback_ = cb; }

    void setMessageCallback(const MessageCallback& cb)
    { messageCallback_ = cb; }

    void setWriteCompleteCallback(const WriteCompleteCallback& cb)
    { writeCompleteCallback_ = cb; }

    void setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark)
    { highWaterMarkCallback_ = cb; highWaterMark_ = highWaterMark; }

    /// Advanced interface
    Buffer* inputBuffer()
    { return &inputBuffer_; }

    Buffer* outputBuffer()
    { return &outputBuffer_; }

    /// Internal use only.
    void setCloseCallback(const CloseCallback& cb)
    { closeCallback_ = cb; }

    // called when TcpServer accepts a new connection
    void connectEstablished();   // should be called only once
    // called when TcpServer has removed me from its map
    void connectDestroyed();  // should be called only once
private:
    enum StateE { kDisconnected, kConnecting, kConnected, kDisconnecting };
    void handleRead(Timestamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();

    void sendInLoop(const StringPiece& message);
    void sendInLoop(const void* message, size_t len);

    void shutdownInLoop();

    void forceCloseInLoop();

    void setState(StateE s) { state_ = s; }

    const char* stateToString() const;

    void startReadInLoop();
    void stopReadInLoop();

private:
    EventLoop* loop_;
    const string name_;
    StateE state_;
    bool reading_;

    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;

    const InetAddress localAddr_;
    const InetAddress peerAddr_;

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
} // namespace muduo::net


#endif //LIBZV_TCP_CONNECTION_H
