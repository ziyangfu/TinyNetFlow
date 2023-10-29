//
// Created by fzy on 23-8-25.
//

#ifndef TINYNETFLOW_SOMEIPHEADER_H
#define TINYNETFLOW_SOMEIPHEADER_H

#include <cstdint>
#include "netflow/someip/interface/EnumType.h"
#include "netflow/someip/interface/TypeDefine.h"

namespace netflow::net::someip {

class SomeIpHeader {
public:
    /** ------- SOME/IP 协议 header ------------------ */
    ServiceId               serviceId_;
    MethodId                methodId_;
    SomeIpLength            length_;
    ClientId                clientId_;
    SessionId               sessionId_;
    ProtocolVersion         protocolVersion_;
    InterfaceVersion        interfaceVersion_;
    SomeIpMessageType       messageType_;
    SomeIpReturnCode        returnCode_;
    /** --------------------------------------------- */
    InstanceId              instanceId_;
    MessageId               messageId_;
    RequestId               requestId_;
public:
    SomeIpHeader();
    /** 拷贝构造 */
    SomeIpHeader(const SomeIpHeader& header);
    ~SomeIpHeader();
};

}  // namespace netflow::net::someip



#endif //TINYNETFLOW_SOMEIPHEADER_H
