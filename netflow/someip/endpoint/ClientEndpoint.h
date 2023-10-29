//
// Created by fzy on 23-9-20.
//

#ifndef TINYNETFLOW_CLIENTENDPOINT_H
#define TINYNETFLOW_CLIENTENDPOINT_H

#include <mutex>
#include <memory>

#include "netflow/someip/endpoint/Endpoint.h"
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




private:


};

} // namespace netflow::net::someip

#endif //TINYNETFLOW_CLIENTENDPOINT_H
