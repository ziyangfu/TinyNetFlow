//
// Created by fzy on 23-11-16.
//

#ifndef TINYNETFLOW_OSADAPTOR_UDSSERVER_H
#define TINYNETFLOW_OSADAPTOR_UDSSERVER_H

#include "IO/ipc/uds/PreDefineUds.h"
#include "IO/ipc/uds/UdsSocket.h"

#include "time/Timestamp.h"

#include <atomic>
#include <functional>
#include <memory>

namespace netflow::osadaptor {

namespace net {
class Channel;
class Acceptor;
class InetAddr;
class EventLoop;
class EventLoopThreadPool;
}  // namespace net

namespace ipc {

/*!
 * \brief 使用 Unix 域套接字这种IPC方式的服务端, receiver */
class UdsServer {
public:
    using MessageCb =  std::function<void (const std::string& message,
                                           time::Timestamp receiveTime)>;
    using ConnectionCb = std::function<void (time::Timestamp receiveTime)>;

private:
    int sockfd_;
    int clientFd_;
    UnixDomainPath path_;
    const std::string unixDomainStringPath_;  /** 必须在 domain 与 port 的后面 */
    net::EventLoop* loop_;
    const std::string name_;
    std::atomic_bool isConnected_;   /** 标识是否使用connect添加了远端地址， 若true则可以使用send */

    std::unique_ptr<net::Channel> channel_;
    std::unique_ptr<net::Channel> connectedChannel_;
    std::shared_ptr<net::EventLoopThreadPool> threadPool_;

    ConnectionCb connectionCallback_;
    MessageCb messageCallback_;/** 消息回调 */;

    static const int kBufferSize;

public:
    UdsServer(net::EventLoop* loop, const std::string& name,
              struct UnixDomainPath path = uds::UnixDomainDefaultPath);
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
    void newConnection(int sockfd, const net::InetAddr& peerAddr);
    void removeConnection();
    void removeConnectionInLoop();

    std::string generateUnixDomainPath();
    void handleRead(time::Timestamp receiveTime);
    void handleClose();
    void handleError();
    void onAccept();
    void sendInLoop(const void *message, size_t len);
    void sendInLoop(const std::string& message);
};

}  //namespace ipc

}  // namespace netflow::osadaptor

#endif //TINYNETFLOW_OSADAPTOR_UDSSERVER_H
