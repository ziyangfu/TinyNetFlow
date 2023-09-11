//
// Created by fzy on 23-8-29.
//

#ifndef TINYNETFLOW_CONSTANTS_H
#define TINYNETFLOW_CONSTANTS_H

#include <cstdint>

/** SOME/IP协议栈中的常量 */
namespace netflow::net::someip {

constexpr std::uint8_t SOMEIP_PROTOCOL_VERSION = 0x01;


constexpr std::size_t MAX_UDP_MESSAGE_SIZE = 1416;
}   // namespace netflow::net::someip

#endif //TINYNETFLOW_CONSTANTS_H
