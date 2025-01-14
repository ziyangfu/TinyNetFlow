//
// Created by fzy on 23-11-7.
//
/*!
 *
 * \brief Unix Domain Socket 客户端
 * \details client 负责创建uds文件
 * */

#ifndef OSADAPTOR_IO_IPC_UDS__UDS_CLIENT_H
#define OSADAPTOR_IO_IPC_UDS__UDS_CLIENT_H

#include <string>
#include <functional>
#include <memory>
#include <atomic>

#include "time/Timestamp.h"
#include "IO/ipc/uds/UdsConstant.h"
#include "IO/ipc/uds/UdsSocket.h"
namespace osadaptor {

namespace net {
class EventLoop;
class Channel;
}  // namespace net
namespace ipc {

class UdsClient {
public:
    using messageCb =  std::function<void (const std::string& message,
                                           time::Timestamp receiveTime)>;
    using ConnectionCb = std::function<void ()>;
private:
    int sockfd_;
    uds::UnixDomainPath path_;
    const std::string unixDomainStringPath_;  /** 必须在 domain 与 port 的后面 */
    net::EventLoop* loop_;
    const std::string name_;
    std::atomic_bool isConnected_;   /** 标识是否使用connect添加了远端地址， 若true则可以使用send */
    std::unique_ptr<net::Channel> channel_;
    messageCb messageCallback_;    /** 消息回调 */;
    ConnectionCb udsConnectionCallback_; /** uds 连接建立回调 */
    ConnectionCb shmConnectionCallback_; /** uds + shm 连接建立回调 */

    static const int kBufferSize;
public:
    UdsClient(net::EventLoop* loop, const std::string& name,
              struct uds::UnixDomainPath path = uds::UnixDomainDefaultPath);
    ~UdsClient();
    void connect();
    void close();
    void send(const std::string& message);
    void send(const char* data, size_t length);
    void sendMemFd(int memFd);
    void setMessageCallback(messageCb cb);
    void setUdsConnectionCallback(ConnectionCb cb);
    void setShmConnectionCallback(ConnectionCb cb);

    int getDomain() const;
    int getPort() const;
    const std::string& getUnixDomainAddr() const;
    int getFd() const;
    const std::string& getName() const;
private:
    std::string generateUnixDomainPath();
    void handleRead(time::Timestamp receiveTime);
    void handleClose();
    void handleError();
    void sendInLoop(const void *message, size_t len);
    void sendInLoop(const std::string& message);
};
}  // namespace ipc
}  // namespace osadaptor



#endif //OSADAPTOR_IO_IPC_UDS__UDS_CLIENT_H
