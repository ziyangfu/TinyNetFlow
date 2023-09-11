//
// Created by fzy on 23-9-10.
//

#ifndef TINYNETFLOW_MESSAGE_H
#define TINYNETFLOW_MESSAGE_H

#include <memory>
#include "src/someip/interface/TypeDefine.h"

namespace netflow::net::someip {

class Payload;

class Message {
public:
    Message() = default;
    virtual ~Message() = default;

    virtual std::shared_ptr<Payload> getPayload() const = 0;
    virtual void setPayload(std::shared_ptr<Payload> payload) = 0;

    /** MessageBase  SOME/IP header */
    /** Message ID */
    virtual MessageId getMessageId() const = 0;
    virtual void setMessageId(MessageId messageId) = 0;

    virtual ServiceId getServiceId() const = 0;
    virtual void setServiceId(ServiceId serviceId) = 0;

    virtual MethodId getMethodId() const = 0;
    virtual void setMethodId(MethodId methodId) = 0;
    /** Request ID */
    virtual RequestId getRequestId() const = 0;
    virtual void setRequestId(RequestId requestId) = 0;

    virtual ClientId getClientId() const = 0;
    virtual void setClientId(ClientId clientId) = 0;

    virtual SessionId getSessionId() const = 0;
    virtual  void setSessionId(SessionId sessionId) = 0;

    /** protocol version */
    virtual ProtocolVersion getProtocolVersion() const = 0;
    virtual void setProtocolVersion(ProtocolVersion protocolVersion) = 0;

    /** interface version */
    virtual InterfaceVersion getInterfaceVersion() const = 0;
    virtual void setInterfaceVersion(InterfaceVersion interfaceVersion) = 0;

    /** message type */
    virtual SomeIpMessageType getMessageType() const = 0;
    virtual void setMessageType(SomeIpMessageType type) = 0;
    /** return code */
    virtual SomeIpReturnCode getReturnCode() const = 0;
    virtual void setReturnCode(SomeIpReturnCode returnCode) = 0;




    virtual InstanceId getInstanceId() const = 0;
    virtual void setInstanceId(InstanceId instanceId) = 0;

};
}  // namespace netflow::net::someip

#endif //TINYNETFLOW_MESSAGE_H
