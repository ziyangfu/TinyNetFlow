//
// Created by fzy on 23-11-7.
//

#ifndef OSADATOR_IO_IPC__UDS_CONSTANT_H
#define OSADATOR_IO_IPC__UDS_CONSTANT_H

#include <string>
//#include "IO/ipc/IpcMediaInfo.h"
namespace osadaptor::ipc {
namespace uds {

struct UnixDomainPath {
    int domain;
    int port;
};

static constexpr int kDefaultDomain = 10;
static constexpr int kDefaultPort = 10;
static struct UnixDomainPath UnixDomainDefaultPath{kDefaultDomain,kDefaultPort };
/** default path is /tmp/netflow_domain_10_port_10 */
static const std::string kUnixDomainPathFirstString = "/tmp/netflow_domain_";
static const std::string kUnixDomainPathSecondString = "_port_";
/** 如果是默认路径，则不需要组装，直接使用以下路径 */
static const std::string kUnixDomainDefaultPathString = "/tmp/netflow_domain_10_port_10.socket";

}  // namespace uds

}  // namespace osadaptor::ipc

#endif //OSADATOR_IO_IPC__UDS_CONSTANT_H
