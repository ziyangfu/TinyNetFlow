//
// Created by fzy on 2024/8/26.
//

#ifndef TINYNETFLOW_OSADAPTOR_SOCKETTYPE_H
#define TINYNETFLOW_OSADAPTOR_SOCKETTYPE_H

#include <cstdint>

namespace osadaptor {
namespace net {

struct SocketFamily {
    std::int32_t value;
};

struct SocketType {
    std::int32_t value;
};

struct SocketProtocol {
    std::int32_t value;

};

}  // namespace net
}  // namespace osadaptor

#endif //TINYNETFLOW_OSADAPTOR_SOCKETTYPE_H
