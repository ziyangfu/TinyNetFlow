//
// Created by fzy on 23-11-7.
//

#ifndef TINYNETFLOW_UDSSOCKET_H
#define TINYNETFLOW_UDSSOCKET_H

#include <string>
#include "netflow/net/UDS/PreDefine.h"

/** Unix Domain Socket */
namespace netflow::net {
class UdsSocket {
public:
    UdsSocket(int domain = uds::kDefaultDomain, int port = uds::kDefaultPort);
    ~UdsSocket();
    int createSocket();
    void bind();
    void connect();

    int getDomain() const;
    int getPort() const;
    int getFd() const;
    const std::string& getLocalAddr() const;
private:
    int domain_;  /** 以 domain 与 port 唯一标识一个域套接字 */
    int port_;
    int sockfd_;
    const std::string localAddr;  /** 必须在 domain 与 port 的后面 */
};
}  // namespace netflow::net



#endif //TINYNETFLOW_UDSSOCKET_H
