//
// Created by fzy on 23-11-7.
//

#ifndef TINYNETFLOW_PREDEFINE_H
#define TINYNETFLOW_PREDEFINE_H

#include <string>

namespace netflow::net::uds {

static const std::string udsAddrFirst = "/tmp/netflow_domain_";  /** /tmp/netflow_domain_10_port_10 */
static const std::string udsAddrSecond = "_port_";
static constexpr int kDefaultDomain = 10;
static constexpr int kDefaultPort = 10;
}  // namespace netflow::net::uds

#endif //TINYNETFLOW_PREDEFINE_H
