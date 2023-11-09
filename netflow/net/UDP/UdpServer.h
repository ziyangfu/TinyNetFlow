//
// Created by fzy on 23-8-22.
//

#ifndef TINYNETFLOW_UDPSERVER_H
#define TINYNETFLOW_UDPSERVER_H

#include <thread>
#include <memory>
#include <functional>
#include <vector>
#include <atomic>

#include "netflow/base/Timestamp.h"
#include "netflow/net/InetAddr.h"
#include "netflow/net/Callbacks.h"
#include "netflow/net/Buffer.h"
#include "netflow/net/Channel.h"
#include "netflow/net/EventLoop.h"
#include "netflow/net/EventLoopThreadPool.h"

namespace netflow::net {

class EventLoop;
class EventLoopThreadPool;

using ThreadInitCallback = std::function<void(EventLoop*)>;

class UdpServer {
public:
    enum class Status : uint8_t {
        kRunning = 1,
        kPaused = 2,
        kStopping = 3,
        kStopped = 4,
    };

    enum class Option : uint8_t {
        kNoReusePort,
        kReusePort
    };

    UdpServer(EventLoop* loop, const InetAddr& addr,
              const std::string& name, Option option = Option::kNoReusePort);
    ~UdpServer();

    const std::string& getIpPort() const { return ipPort_; }
    const std::string& getName() const { return name_; }
    EventLoop* getLoop() const { return loop_; }
    void setThreadInitCallback(const ThreadInitCallback& cb) { threadInitCallback_ = cb; }
    std::shared_ptr<EventLoopThreadPool> getThreadPoolPtr() { return threadPool_; }
    void setMessageCallback(const MessageCallbackUdp& cb) { messageCallback_ = cb; }
    void setEventLoopThreadPool(const std::shared_ptr<EventLoopThreadPool>& pool) {threadPool_ = pool; }

    void setThreadNum(int numThreads);
    void start();
    void close();
    bool send(const std::string& message);
    bool send(const char* data, size_t length);

    /** ------------------------------------ UDP 多播部分 -----------------------------------------------*/
private:
    void handleRead(base::Timestamp receiveTime);
    void handleClose();
    void handleError();

private:
    int sockfd_;
    EventLoop* loop_;
    InetAddr localAddr_;
    const std::string name_;
    Option option_;
    std::unique_ptr<Channel> channel_;
    Buffer receiveBuffer;
    Status status_;
    const std::string ipPort_;
    std::shared_ptr<EventLoopThreadPool> threadPool_;


    MessageCallbackUdp messageCallback_;
    ThreadInitCallback threadInitCallback_;
    std::atomic_bool started_;

    static const int kBufferSize;
};
} // namespace netflow::net



#endif //TINYNETFLOW_UDPSERVER_H
