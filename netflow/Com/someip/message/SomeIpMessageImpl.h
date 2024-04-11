//
// Created by fzy on 23-8-22.
//

#ifndef TINYNETFLOW_SOMEIPMESSAGEIMPL_H
#define TINYNETFLOW_SOMEIPMESSAGEIMPL_H

#include <stdint.h>
#include <vector>
#include <memory>

#include "SomeIpHeader.h"
#include "SomeIpPayload.h"
#include "netflow/Com/someip/interface/Message.h"


/** 表示一条SOME/IP消息 */
namespace netflow::net::someip {

class SomeIpMessageImpl : public Message {
public:
    SomeIpMessageImpl();
    ~SomeIpMessageImpl();

    const SomeIpHeader & getHeader() const override;
    std::shared_ptr<Payload> getPayload() const override;
    void setPayload(std::shared_ptr<Payload> payload) override;

    MessageId getMessageId() const override;
    void setMessageId(MessageId messageId) override;

    RequestId getRequestId() const override;
    void setRequestId(RequestId requestId) override;

    InstanceId getInstanceId() const override;
    void setInstanceId(InstanceId instanceId) override;

    /** ------------------------SOME/IP Header ---------------------------------*/
    ServiceId getServiceId() const override;
    void setServiceId(ServiceId serviceId) override;

    MethodId getMethodId() const override;
    void setMethodId(MethodId methodId) override;

    SomeIpLength getLength() const override;
    void setLength(SomeIpLength length) override;

    ClientId getClientId() const override;
    void setClientId(ClientId clientId) override;

    SessionId getSessionId() const override;
    void setSessionId(SessionId sessionId) override;

    ProtocolVersion getProtocolVersion() const override;
    void setProtocolVersion(ProtocolVersion protocolVersion) override;

    InterfaceVersion getInterfaceVersion() const override;
    void setInterfaceVersion(InterfaceVersion interfaceVersion) override;

    SomeIpMessageType getMessageType() const override;
    void setMessageType(SomeIpMessageType type) override;

    SomeIpReturnCode getReturnCode() const override;
    void setReturnCode(SomeIpReturnCode returnCode) override;

private:
    /**
     * SomeIpHeader
     * Payload
     * */
     SomeIpHeader header_;
     std::shared_ptr<Payload> payloadPtr;
};


} // namespace netflow::net::someip



#endif //TINYNETFLOW_SOMEIPMESSAGEIMPL_H
