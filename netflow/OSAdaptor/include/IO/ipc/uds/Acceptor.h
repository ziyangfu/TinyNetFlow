
/*!
 * 若是uds连接，则直接调用onAccept连接即可
 * 若是shm连接，则需要先调用onAccept连接，再调用onShmAccept连接，通过传递的memfd创建共享内存映射
 * */

#ifndef OSADAPTOR_IO_IPC_UDS__ACCEPTOR_H
#define OSADAPTOR_IO_IPC_UDS__ACCEPTOR_H

#include <functional>
#include "IO/reactor/Channel.h"

#if 0

namespace osadaptor {
namespace net {

class EventLoop;
class InetAddr;

} // namespace net

namespace ipc {

class Acceptor {
public:
    Acceptor();
    ~Acceptor();
    using NewConnectionCallback = std::function<void (int sockfd, const net::InetAddr&)>;
    using ConnectionCb = std::function<void(int)>;
    using MessageCb = std::function<void(const std::string&)>;
    using ShmConnectionCb = std::function<void(int, int)>;
    using UdsConnectionCb = std::function<void(int)>;
    using ShmMessageCb = std::function<void(const std::string&, int)>;
    using UdsMessageCb = std::function<void(const std::string&)>;

    void setNewConnectionCallback(const NewConnectionCallback& cb);

private:
    void listen();
    void handleReactorEventRead();

private:
    net::EventLoop *loop_;
    int udsAcceptFd_;
    net::Channel acceptChannel_;
    bool isShmConnection_{false};
};

}  // namespace ipc
} // namespace osadaptor


#endif

#endif //OSADAPTOR_IO_IPC_UDS__ACCEPTOR_H
