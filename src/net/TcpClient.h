//
// Created by fzy on 23-5-24.
//

#ifndef TINYNETFLOW_TCPCLIENT_H
#define TINYNETFLOW_TCPCLIENT_H
#include <memory>
#include <mutex>
#include "TcpConnection.h"

namespace netflow::net {

class Connector;
using ConnectorPtr = std::shared_ptr<Connector>;

class TcpClient {
public:
    TcpClient(EventLoop* loop, const InetAddr& serverAddr, const std::string& name);
    ~TcpClient();

    void connect();
    void disconnect();
    void stop();

    TcpConnectionPtr getTcpConnectionPtr() const
    {
        std::unique_lock<std::mutex> lock(mutex_);  /**  为什么要加锁 */
        return connection_;
    }
    bool retry() const { return retry_; }
    void enableRetry() { retry_ = true; }

    const std::string& getName() const { return name_; }

    void setConnectionCallback(ConnectionCallback& cb) { connectionCallback_ = std::move(cb); }
    void setMessageCallback(MessageCallback& cb) { messageCallback_ = std::move(cb); }
    void setWriteCompleteCallback(WriteCompleteCallback& cb) { writeCompleteCallback_ = std::move(cb); }
private:
    void newConnection(int sockfd);
    void removeConnection(const TcpConnectionPtr& conn);
private:
    EventLoop* loop_;
    ConnectorPtr connector_;
    const std::string name_;

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;

    std::atomic_bool retry_;
    std::atomic_bool connect_;

    int nextConnId_;
    std::mutex mutex_;
    TcpConnectionPtr connection_;

};
}  // namespace netflow::net



#endif //TINYNETFLOW_TCPCLIENT_H
