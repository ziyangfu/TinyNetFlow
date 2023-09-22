//
// Created by fzy on 23-9-20.
//

#ifndef TINYNETFLOW_ENDPOINTDESCRIPTION_H
#define TINYNETFLOW_ENDPOINTDESCRIPTION_H

#include <string>

#include "src/net/InetAddr.h"

namespace netflow::net::someip {
/** 描述一个 Endpoint */
class EndpointDescription {
public:
    EndpointDescription();
    ~EndpointDescription();

private:
    /** TODO:  底层IP地址方面，使用 InetAddr */
    net::InetAddr inetAddr_;


    std::string addr_;
    uint16_t port_;
    uint16_t remotePort_;
    bool isReliable_;
};

}  // namespace netflow::net::someip



#endif //TINYNETFLOW_ENDPOINTDESCRIPTION_H
