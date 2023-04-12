//
// Created by fzy on 23-4-12.
//
/** TCP客户端的连接器 */
#ifndef LIBZV_CONNECTOR_H
#define LIBZV_CONNECTOR_H

#include "../base/noncopyable.h"
#include "inet_address.h"

#include <functional>
#include <memory>

namespace muduo::net {

class Channel;
class EventLoop;

class Connector : Noncopyable,
                  public std::enable_shared_from_this<Connector>  /** TODO 这个是用来干啥的 */
{
public:
    using NewConnectionCallback = std::function<void (int sockfd)>;

    Connector(EventLoop* loop, const InetAddress& serverAddr);
    ~Connector();

    void setNewConnectionCallback(const NewConnectionCallback& cb)
    { newConnectionCallback_ = cb; }

    void start();  // can be called in any thread
    void restart();  // must be called in loop thread
    void stop();  // can be called in any thread

    const InetAddress& serverAddress() const { return serverAddr_; }
private:
    enum States { kDisconnected, kConnecting, kConnected };
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

private:
    static const int kMaxRetryDelayMs = 30*1000;
    static const int kInitRetryDelayMs = 500;

    EventLoop* loop_;
    InetAddress serverAddr_;
    bool connect_;
    States state_;
    std::unique_ptr<Channel> channel_;
    NewConnectionCallback newConnectionCallback_;
    int retryDelayMs_;

};
} // namespace muduo::net



#endif //LIBZV_CONNECTOR_H
