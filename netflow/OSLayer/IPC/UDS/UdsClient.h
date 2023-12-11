//
// Created by fzy on 23-11-7.
//

#ifndef TINYNETFLOW_UDSSOCKET_H
#define TINYNETFLOW_UDSSOCKET_H

#include <string>
#include <functional>
#include <memory>

#include "netflow/net/Channel.h"
#include "netflow/base/Timestamp.h"
#include "netflow/net/EventLoop.h"

#include "PreDefineUds.h"
#include "UdsSocketOps.h"

/** Unix Domain Socket 客户端 */
namespace netflow::net {
class UdsClient {
public:
    using messageCb =  std::function<void (const std::string& message,
                                           netflow::base::Timestamp receiveTime)>;

    UdsClient(EventLoop* loop, const std::string& name,
              struct uds::UnixDomainPath path = uds::UnixDomainDefaultPath);
    ~UdsClient();
    void connect();
    void close();
    void send(const std::string& message);
    void send(const char* data, size_t length);
    void setMessageCallback(messageCb cb);
    int getDomain() const;
    int getPort() const;
    const std::string& getUnixDomainAddr() const;
    int getFd() const;
    const std::string& getName() const;
private:
    std::string generateUnixDomainPath();
    //std::string formatString(std::span<char> buffer, const char* format, ...) {
    // std::span need C++20, this project is based on C++17
    // va_list args;
    // va_start(args, format);
    // int const strBytes {::vsnprintf(buffer.data, buffer.size, format, args)}
    // va_end(args);
    // std::string(buffer.data, static_cast<size_t>(strBytes));
    // }

    void handleRead(base::Timestamp receiveTime);
    void handleClose();
    void handleError();
    void sendInLoop(const void *message, size_t len);
    void sendInLoop(const std::string& message);
private:
    int sockfd_;
    uds::UnixDomainPath path_;
    const std::string unixDomainStringPath_;  /** 必须在 domain 与 port 的后面 */
    EventLoop* loop_;
    const std::string name_;
    std::atomic_bool isConnected_;   /** 标识是否使用connect添加了远端地址， 若true则可以使用send */
    std::unique_ptr<Channel> channel_;

    messageCb messageCallback_;/** 消息回调 */;

    static const int kBufferSize;
};
}  // namespace netflow::net



#endif //TINYNETFLOW_UDSSOCKET_H
