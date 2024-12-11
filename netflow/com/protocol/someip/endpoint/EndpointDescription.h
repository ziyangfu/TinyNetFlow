//
// Created by fzy on 23-9-20.
//

#ifndef TINYNETFLOW_ENDPOINTDESCRIPTION_H
#define TINYNETFLOW_ENDPOINTDESCRIPTION_H

#include <string>

#include "netflow/net//InetAddr.h"

namespace netflow::net::someip {
/** 描述一个 Endpoint */
class EndpointDescription {
public:
    EndpointDescription(const std::string& address, uint16_t port, bool isUseTcp);
    ~EndpointDescription();

    const std::string& getAddress() const;
    void setAddress(const std::string& address);

    uint16_t getPort() const;
    void setPort(uint16_t port);

    uint16_t getRemotePort() const;
    void setRemotePort(uint16_t port);

    bool isReliable() const;
    void setReliable(bool isUseTcp);
private:
    std::string addr_;   /** 本地 IP 地址 */
    uint16_t port_;      /** 本地端口 */
    uint16_t remotePort_;/** 远端端口 */
    bool isReliable_;    /** 使用 TCP/UDP */
};

}  // namespace netflow::net::someip



#endif //TINYNETFLOW_ENDPOINTDESCRIPTION_H
