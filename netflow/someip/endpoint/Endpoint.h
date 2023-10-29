//
// Created by fzy on 23-9-20.
//

#ifndef TINYNETFLOW_ENDPOINT_H
#define TINYNETFLOW_ENDPOINT_H

#include <stdint.h>

namespace netflow::net::someip {

/** Endpoint 接口类 */
class Endpoint {
public:
    Endpoint() = default;
    virtual ~Endpoint() = default;

    virtual void start() = 0;
    virtual void stop() = 0;

    virtual bool isConnected() const = 0;

    virtual bool send() = 0;
    virtual bool sendTo() = 0;

    virtual void enableMagicCookies() = 0; /** 作用是什么？ */
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
