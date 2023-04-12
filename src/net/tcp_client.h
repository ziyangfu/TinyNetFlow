//
// Created by fzy on 23-3-13.
//

/*! TCP 客户端 */
#ifndef LIBZV_TCP_CLIENT_H
#define LIBZV_TCP_CLIENT_H

#include "../base/mutex.h"
#include "tcp_connection.h"

namespace muduo::net {

class Connector;
using ConnectorPtr = std::shared_ptr<Connector>;

class TcpClient : Noncopyable {
public:
    TcpClient(EventLoop* loop, const InetAddress& serverAddr, const string& nameArg);
    ~TcpClient();

    void connect();
    void disconnect();
    void stop();

    TcpConnectionPtr connection() const {
        MutexLockGuard lock(mutex_);  /** 为什么要加锁 */
        return connection_;
    }
    EventLoop* getLoop() const { return loop_; }
    bool retry() const { return retry_; }
    void enableRetry() { retry_ = true; }

    const string& name() const
    { return name_; }

    void setConnectionCallback(ConnectionCallback cb)
    { connectionCallback_ = std::move(cb); }

    /// Set message callback.
    /// Not thread safe.
    void setMessageCallback(MessageCallback cb)
    { messageCallback_ = std::move(cb); }

    /// Set write complete callback.
    /// Not thread safe.
    void setWriteCompleteCallback(WriteCompleteCallback cb)
    { writeCompleteCallback_ = std::move(cb); }

private:
    void newConnection(int sockfd);
    void removeConnection(const TcpConnectionPtr& conn);
private:
    EventLoop* loop_;
    ConnectorPtr connector_;
    const string name_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    bool retry_;
    bool connect_;

    int nextConnId_;
    mutable MutexLock mutex_;
    TcpConnectionPtr connection_ GUARDED_BY(mutex_);
};
} // namespace muduo::net

#endif //LIBZV_TCP_CLIENT_H
