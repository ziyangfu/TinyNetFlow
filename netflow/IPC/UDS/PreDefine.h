//
// Created by fzy on 23-11-7.
//

#ifndef TINYNETFLOW_PREDEFINE_H
#define TINYNETFLOW_PREDEFINE_H

#include <string_view>

namespace netflow::net::uds {

static const std::string udsAddrFirst = "/tmp/netflow-domain-";
static const std::string udsAddrSecond = "-port-";
static constexpr int kDefaultDomain = 10;
static constexpr int kDefaultPort = 10;
}  // namespace netflow::net::uds

#endif //TINYNETFLOW_PREDEFINE_H
