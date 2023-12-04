//
// Created by fzy on 23-11-16.
//

#ifndef TINYNETFLOW_UDSSERVER_H
#define TINYNETFLOW_UDSSERVER_H

#include "netflow/IPC/UDS/PreDefineUds.h"
#include "netflow/IPC/UDS/UdsSocketOps.h"
#include "netflow/base/Timestamp.h"

#include <atomic>
#include <functional>
#include <memory>

namespace netflow::net {


class Channel;
class Acceptor;
class InetAddr;
class EventLoop;
class EventLoopThreadPool;
/*!
 * \brief 使用 Unix 域套接字这种IPC方式的服务端, receiver */
class UdsServer {
public:
    using MessageCb =  std::function<void (const std::string& message,
                                           netflow::base::Timestamp receiveTime)>;
    using ConnectionCb = std::function<void (netflow::base::Timestamp receiveTime)>;

    UdsServer(EventLoop* loop, const std::string& name,
              struct uds::UnixDomainPath path = uds::UnixDomainDefaultPath);
    ~UdsServer();

    void start();
    void stop();
    void close();

    void bind();
    void listen();
    int accept();
    void send(const std::string& message);

    void setMessageCallback(MessageCb cb);
    void setConnectionCallback(ConnectionCb cb);

    void setThreadNums(int threadNum);
private:
    void newConnection(int sockfd, const InetAddr& peerAddr);
    void removeConnection();
    void removeConnectionInLoop();

    std::string generateUnixDomainPath();
    void handleRead(base::Timestamp receiveTime);
    void handleClose();
    void handleError();
    void onAccept();
    void sendInLoop(const void *message, size_t len);
    void sendInLoop(const std::string& message);


private:
    int sockfd_;
    int clientFd_;
    uds::UnixDomainPath path_;
    const std::string unixDomainStringPath_;  /** 必须在 domain 与 port 的后面 */
    EventLoop* loop_;
    const std::string name_;
    std::atomic_bool isConnected_;   /** 标识是否使用connect添加了远端地址， 若true则可以使用send */

    std::unique_ptr<Channel> channel_;
    std::unique_ptr<Channel> connectedChannel_;
    std::shared_ptr<EventLoopThreadPool> threadPool_;

    ConnectionCb connectionCallback_;
    MessageCb messageCallback_;/** 消息回调 */;

    static const int kBufferSize;
};

}
#endif //TINYNETFLOW_UDSSERVER_H
