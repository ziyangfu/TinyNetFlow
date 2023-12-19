//
// Created by fzy on 23-5-24.
//

#ifndef TINYNETFLOW_OSADAPTOR_TCPSERVER_H
#define TINYNETFLOW_OSADAPTOR_TCPSERVER_H

#include "TcpConnection.h"
#include "Callbacks.h"

#include <map>
#include <functional>
#include <memory>
#include <atomic>

namespace netflow::net {

class Acceptor;
class EventLoop;
class EventLoopThreadPool;
/*!
 * \brief TCP服务端， 支持单IO线程模型与多IO线程模型
 *        接口类 */
class TcpServer {
private:
    using ConnectionMap = std::map<std::string, TcpConnectionPtr>;

    EventLoop* loop_;
    const std::string ipPort_;
    const std::string name_;

    std::unique_ptr<Acceptor> acceptor_;
    std::shared_ptr<EventLoopThreadPool> threadPool_;

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;

    ThreadInitCallback threadInitCallback_;

    std::atomic_bool started_;
    int nextConnId_;
    ConnectionMap connections_;
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;
    enum Option {
        kNoReusePort,
        kReusePort
    };
    TcpServer(EventLoop* loop, const InetAddr& listenAddr, const std::string& name, Option option = kNoReusePort);
    ~TcpServer();
    const std::string& getIpPort() const { return ipPort_; }
    const std::string& getName() const { return name_; }
    EventLoop* getLoop() const { return loop_; }

    void setThreadNum(int numThreads);
    void setThreadInitCallback(const ThreadInitCallback& cb) { threadInitCallback_ = cb; }
    std::shared_ptr<EventLoopThreadPool> getThreadPoolPtr() { return threadPool_; }

    void start();

    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback& cb) { writeCompleteCallback_ = cb; }

private:
    void newConnection(int sockfd, const InetAddr& peerAddr);
    void removeConnection(const TcpConnectionPtr& conn);
    void removeConnectionInLoop(const TcpConnectionPtr& conn);

};

} // namespace netflow::net



#endif //TINYNETFLOW_OSADAPTOR_TCPSERVER_H
