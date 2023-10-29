//
// Created by fzy on 23-9-20.
//

#ifndef TINYNETFLOW_CLIENTENDPOINTTCP_H
#define TINYNETFLOW_CLIENTENDPOINTTCP_H

#include "netflow/someip/endpoint/ClientEndpoint.h"

namespace netflow::net::someip {
/** 底层基于TCP协议的Endpoint */
class ClientEndpointTcp : public ClientEndpoint {
public:
    ClientEndpointTcp();
    ~ClientEndpointTcp();

    void connect();

private:

};

} // namespace netflow::net::someip
#endif //TINYNETFLOW_CLIENTENDPOINTTCP_H
