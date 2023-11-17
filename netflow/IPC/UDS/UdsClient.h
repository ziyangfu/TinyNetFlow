//
// Created by fzy on 23-11-7.
//

#ifndef TINYNETFLOW_UDSSOCKET_H
#define TINYNETFLOW_UDSSOCKET_H

#include <string>
#include "PreDefine.h"
#include "netflow/IPC/UDS/UdsSocketOps.h"

/** Unix Domain Socket */
namespace netflow::net {
class UdsClient {
public:
    UdsClient(struct uds::UnixDomainPath path = uds::UnixDomainDefaultPath);
    ~UdsClient();
    void bind();
    void connect();

    int getDomain() const;
    int getPort() const;
    const std::string& getUnixDomainAddr() const;

    int getFd() const;
private:
    const std::string generateUnixDomainPath();
private:
    int sockfd_;
    uds::UnixDomainPath path_;
    const std::string unixDomainPath_;  /** 必须在 domain 与 port 的后面 */
};
}  // namespace netflow::net



#endif //TINYNETFLOW_UDSSOCKET_H
