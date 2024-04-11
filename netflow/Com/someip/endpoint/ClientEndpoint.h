//
// Created by fzy on 23-9-20.
//

#ifndef TINYNETFLOW_CLIENTENDPOINT_H
#define TINYNETFLOW_CLIENTENDPOINT_H

#include <mutex>
#include <memory>

#include "Endpoint.h"
namespace netflow::net::someip {

class ClientEndpoint : public Endpoint,
                       public std::enable_shared_from_this<ClientEndpoint>
{
public:
    ClientEndpoint();
    virtual ~ClientEndpoint();

    bool send() override;
    bool sendTo() override;
    bool isConnected() const override;
    void start() override;
    void stop() override;

    void restart();
    bool flush(); /** 立即发送 */
    bool isClient();

public:
    virtual void connect() = 0;
    virtual void receive() = 0;
protected:
    virtual void sendQueued() = 0;  /** 写入队列，等待后续发送 */
private:
    bool isConnected_;
    uint32_t connectTimeout_;



};

} // namespace netflow::net::someip

#endif //TINYNETFLOW_CLIENTENDPOINT_H
