//
// Created by fzy on 2025/1/6.
//
#ifndef OSADAPTOR_IO_IPC__IPC_CONNECTION_H
#define OSADAPTOR_IO_IPC__IPC_CONNECTION_H

#include <string>
#include <memory>
#include <atomic>


/*!
 * \brief 用户建立连接后，发送与接收消息全部使用 IpcConnection
 * \private ？
 * */
namespace osadator::ipc {

class IpcConnection : public std::enable_shared_from_this<IpcConnection>{
private:
    enum class IpcStateE : uint8_t {
        kDisconnected,
        kConnecting,
        kConnected,
        kDisconnecting
    };
    const std::string name_;
    std::atomic<IpcStateE> state_;
    bool reading;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    HighWaterMarkCallback highWaterMarkCallback_;
    CloseCallback closeCallback_;
public:
    IpcConnection() = default;
    ~IpcConnection() = default;
    void send(std::string data);
    std::string receive();
    bool isConnected() const { return state_ == StateE::kConnected; }
    bool isDisconnected() const { return state_ == StateE::kDisconnected; }
    void shutdown();
    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback& cb) { writeCompleteCallback_ = cb; }
    void setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark) {
        highWaterMarkCallback_ = cb;
        highWaterMark_ = highWaterMark;
    }
    /*!
 * \private 内部使用 */
    void setCloseCallback(const CloseCallback& cb) { closeCallback_ = cb; }
    /*!
 * \brief 供 TcpServer 接收新连接时使用 */
    void connectEstablished();
    void connectDestroyed();
private:
    void handleReadEvent(time::Timestamp receiveTime);
    void handleWriteEvent();
    void handleCloseEvent();
    void handleErrorEvent();
};  // class IpcConnection

using IpcConnectionPtr = std::shared_ptr<IpcConnection>;

}  // namespace osadator::ipc


#endif //OSADAPTOR_IO_IPC__IPC_CONNECTION_H
