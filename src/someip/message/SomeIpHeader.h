//
// Created by fzy on 23-8-25.
//

#ifndef TINYNETFLOW_SOMEIPHEADER_H
#define TINYNETFLOW_SOMEIPHEADER_H

#include <cstdint>
#include "src/someip/SomeIpProtocol.h"
#include "src/someip/interface/TypeDefine.h"

namespace netflow::net::someip{

class SomeIpHeader {
public:
    ServiceId               serviceId_;
    MethodId                methodId_;
    SomeIpLength            length_;
    ClientId                clientId_;
    SessionId               sessionId_;
    ProtocolVersion         protocolVersion_;
    InterfaceVersion        interfaceVersion_;
    SomeIpMessageType       messageType_;
    SomeIpReturnCode        returnCode_;
public:
    SomeIpHeader();
    /** 拷贝构造 */
    SomeIpHeader(const SomeIpHeader& header);
    ~SomeIpHeader();

    MessageId getMessageId();
    void setMessageId(ServiceId serviceId, MethodId methodId);
    RequestId getRequestId();
    void setRequestId(ClientId clientId, SessionId sessionId);
private:
    MessageId messageId_;
    RequestId requestId_;

};

}  // namespace netflow::net::someip



#endif //TINYNETFLOW_SOMEIPHEADER_H
