//
// Created by fzy on 23-5-17.
//

#ifndef TINYNETFLOW_OSADAPTOR_CONNECTOR_H
#define TINYNETFLOW_OSADAPTOR_CONNECTOR_H

#include "InetAddr.h"
#include <functional>
#include <memory>
#include <atomic>

namespace netflow::net {

class Channel;
class EventLoop;
/** 客户端接收器 */
class Connector : public std::enable_shared_from_this<Connector> { /** this指针由智能指针管理 */
private:
    static const int kMaxRetryDelayMs = 30*1000;
    static const int kInitRetryDelayMs = 500;
    enum States { kDisconnected, kConnecting, kConnected };
    EventLoop* loop_;
    InetAddr serverAddr_;
    std::atomic_bool connect_;
    std::atomic<States> state_;
    std::unique_ptr<Channel> channel_;
    NewConnectionCallback newConnectionCallback_;
    int retryDelayMs_;
public:
    using NewConnectionCallback = std::function<void (int sockfd)>;
    Connector(EventLoop* loop, const InetAddr& serverAddr);
    ~Connector();

    void setNewConnectionCallback(const NewConnectionCallback& cb)
    { newConnectionCallback_ = cb; }

    void start();  // can be called in any thread
    void restart();  // must be called in loop thread
    void stop();  // can be called in any thread

private:
    void setState(States s) { state_ = s; }
    void startInLoop();
    void stopInLoop();
    void connect();
    void connecting(int sockfd);
    void handleWrite();
    void handleError();
    void retry(int sockfd);
    int removeAndResetChannel();
    void resetChannel();
};
} // namespace netflow::net



#endif //TINYNETFLOW_OSADAPTOR_CONNECTOR_H
