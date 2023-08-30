//
// Created by fzy on 23-8-30.
//

#ifndef TINYNETFLOW_TYPEDEFINE_H
#define TINYNETFLOW_TYPEDEFINE_H

#include <cstdint>
/** SOME/IP协议的各种类型定义 */

namespace netflow::net::someip {
    using MessageId         = uint32_t;
    using ServiceId         = uint16_t;
    using MethodId          = uint16_t;
    using EventId           = uint16_t;

    using InstanceId        = uint16_t;
    using EventgroupId      = uint16_t;
    
    using MajorVersion      = uint8_t;
    using MinorVersion      = uint32_t;
    
    using SomeIpTtl         = uint32_t;
    
    using RequestId         = uint32_t;
    using SessionId         = uint16_t;
    using ClientId          = uint16_t;

    using SomeIpLength      = uint32_t;

    using ProtocolVersion   = uint8_t;
    using InterfaceVersion  = uint8_t;


}  // namespace netflow::net::someip

#endif //TINYNETFLOW_TYPEDEFINE_H
