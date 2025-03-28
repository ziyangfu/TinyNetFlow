//
// Created by fzy on 23-9-20.
//

#ifndef TINYNETFLOW_ENDPOINT_H
#define TINYNETFLOW_ENDPOINT_H

#include <stdint.h>

namespace netflow::net::someip {

/*!
 * \brief Endpoint 接口类
 * \details:
 * 1. endpoint分为 客户端 / 服务端，可分别使用以下三种协议
 *                      1. LOCAL：使用UNIX域套接字
 *                      2. TCP
 *                      3. UDP
 * 2. 一个app可以有多个endpoint，当使用UDS协议时，则使用UDS endpoint通信，当使用
 *    TCP时，则实例化并使用TCP endpoint通信
 *                */
class Endpoint {
public:
    Endpoint() = default;
    virtual ~Endpoint() = default;

    virtual void start() = 0;
    virtual void stop() = 0;

    virtual bool isConnected() const = 0;

    virtual bool send() = 0;
    virtual bool sendTo() = 0;

    virtual void receive() = 0;

    virtual void join() = 0;
    virtual void leave() = 0;

    virtual void addMulticast() = 0;
    virtual void removeMulticast() = 0;

    virtual bool getRemoteAddr() const = 0;
    virtual uint16_t getLocalPort() const = 0;
    virtual uint16_t getRemotePort() const = 0;

    virtual bool isReliable() const = 0;
    virtual bool isLocal() const = 0;
};

}  // namespace netflow::net::someip

#endif //TINYNETFLOW_ENDPOINT_H
