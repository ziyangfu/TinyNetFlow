//
// Created by fzy on 23-8-29.
//

#ifndef TINYNETFLOW_CONSTANTS_H
#define TINYNETFLOW_CONSTANTS_H

#include <cstdint>
#include <string_view>

/** SOME/IP协议栈中的常量 */
namespace netflow::net::someip {

constexpr std::string_view UdsBasePath = "/tmp/someip-";   /** 编译期字符串常量 */
// constexpr char* UdsBasePathXx = "/tmp/someip-";

constexpr std::uint8_t kSomeIpProtocolVersion = 0x01U;

constexpr std::size_t kMaxUdsMessageSize = 32768;  /** 使用UNIX域套接字时的最大消息长度 */
constexpr std::size_t kMaxTcpMessageSize = 4095;
constexpr std::size_t kMaxUdpMessageSize = 1416;

/** ------------------------------- SOME/IP-SD ----------------------------------*/
constexpr std::uint16_t kSdServiceId = 0xFFFFU;
constexpr std::uint16_t kSdMethodId = 0x8100U;
constexpr std::uint16_t kSdInstanceId = 0x0000U;
constexpr std::uint16_t kSdClientId = 0x0000U;

constexpr std::size_t kSdDefaultMinInitialDelay = 0;
constexpr std::size_t kSdDefaultMaxInitialDelay = 3000;   /** ms ? */
constexpr std::size_t kSdDefaultRepetitionBaseDelay = 10;
constexpr std::size_t kSdDefaultRepetitionMax = 5;
constexpr std::size_t kSdDefaultCyclicOfferDelay = 1000;
constexpr std::size_t kSdDefaultCyclicRequestDelay = 2000;


/** major version and minor version */

}   // namespace netflow::net::someip

#endif //TINYNETFLOW_CONSTANTS_H
