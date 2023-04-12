//
// Created by fzy on 23-3-13.
//

/*! TCP 服务端 */
#ifndef LIBZV_TCP_SERVER_H
#define LIBZV_TCP_SERVER_H

#include "../base/atomic.h"
#include "../base/types.h"
#include "tcp_connection.h"

#include <map>

namespace muduo::net {

class Acceptor;
class EventLoop;
class EventLoopThreadPool;

class TcpServer : Noncopyable {
public:
    using ThreadInitCallback = std::function<void (EventLoop*)>;
    enum Option
    {
        kNoReusePort,
        kReusePort,
    };

    TcpServer(EventLoop* loop,
              const InetAddress& listenAddr,
              const string& nameArg,
              Option option = kNoReusePort);
    ~TcpServer();  // force out-line dtor, for std::unique_ptr members.

    const string& ipPort() const { return ipPort_; }
    const string& name() const { return name_; }
    EventLoop* getLoop() const { return loop_; }

    void setThreadNum(int numThreads);
    void setThreadInitCallback(const ThreadInitCallback& cb) { threadInitCallback_ = cb; }

    std::shared_ptr<EventLoopThreadPool> threadPool() { return threadPool_; }

    /// Starts the server if it's not listening.
    ///
    /// It's harmless to call it multiple times.
    /// Thread safe.
    void start();

    /// Set connection callback.
    /// Not thread safe.
    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }

    /// Set message callback.
    /// Not thread safe.
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }

    /// Set write complete callback.
    /// Not thread safe.
    void setWriteCompleteCallback(const WriteCompleteCallback& cb) { writeCompleteCallback_ = cb; }

private:
    void newConnection(int sockfd, const InetAddress& peerAddr);

    void removeConnection(const TcpConnectionPtr& conn);

    void removeConnectionInLoop(const TcpConnectionPtr& conn);
private:
    using ConnectionMap = std::map<string, TcpConnectionPtr>;

    EventLoop* loop_;
    const string ipPort_;
    const string name_;
    std::unique_ptr<Acceptor> acceptor_;
    std::shared_ptr<EventLoopThreadPool> threadPool_;

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;

    ThreadInitCallback threadInitCallback_;
    AtomicInt32 started_;

    int nextConnId_;
    ConnectionMap connections_;

};
} // namespace muduo::net

#endif //LIBZV_TCP_SERVER_H
