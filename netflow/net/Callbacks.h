//
// Created by fzy on 23-5-17.
//

#ifndef TINYNETFLOW_CALLBACKS_H
#define TINYNETFLOW_CALLBACKS_H

/** 集合上层需要的回调 */

#include <functional>
#include <memory>
#include "netflow/base/Timestamp.h"

namespace netflow::net {
    class Buffer;
    class TcpConnection;
    using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

    using TimerCallback = std::function<void()>;
    using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;
    using CloseCallback = std::function<void(const TcpConnectionPtr&)>;
    using WriteCompleteCallback = std::function<void(const TcpConnectionPtr&)>;
    using HighWaterMarkCallback = std::function<void(const TcpConnectionPtr&, size_t)>;
    using MessageCallback = std::function<void(const TcpConnectionPtr& conn, Buffer* buffer,
                                                netflow::base::Timestamp receiveTime)>;
    /*!
     * \brief UDP消息回调
     * \param message : 回调消息
     * \param remoteAddr ： 发送者的IP地址与端口
     * \param receiveTime : 接收时间
     * */
    using MessageCallbackUdp = std::function<void (const std::string& message, const InetAddr& remoteAddr,
                                             netflow::base::Timestamp receiveTime)>;

    /** 实现在 TcpConnection.cpp */
    void defaultConnectionCallback(const TcpConnectionPtr& conn);
    void defaultMessageCallback(const TcpConnectionPtr& conn,
                                Buffer* buffer,
                                netflow::base::Timestamp receiveTime);

} // namespace netflow::net


#endif //TINYNETFLOW_CALLBACKS_H
