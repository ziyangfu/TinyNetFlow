//
// Created by fzy on 23-8-22.
//

#ifndef TINYNETFLOW_SOMEIPMESSAGEIMPL_H
#define TINYNETFLOW_SOMEIPMESSAGEIMPL_H

#include <stdint.h>
#include <vector>
#include <memory>

#include "src/someip/SomeIpHeader.h"
#include "src/someip//SomeIpPayload.h"
#include "src/someip/interface/Message.h"


/** 表示一条SOMEIP消息 */
namespace netflow::net::someip {

class SomeIpMessageImpl : public Message {
public:
    void setPayload();
    void getPayload();
    SomeIpHeader& getHeader() {return header_; }

    MessageId getMessageId() const override;
    void setMessageId(MessageId messageId) override;

    ServiceId getServiceId() const override;
    void setServiceId(ServiceId serviceId) override;

    InstanceId getInstanceId() const override;
    void setInstanceId(InstanceId instanceId) override;

    MethodId getMethodId() const override;
    void setMethodId(MethodId methodId) override;

    SessionId getSessionId() const override;
    void setSessionId(SessionId sessionId) override;



private:
    /**
     * SomeIpHeader
     * Payload
     * */
     SomeIpHeader header_;
     std::shared_ptr<SomeIpPayload> payloadPtr;
};


} // namespace netflow::net::someip



#endif //TINYNETFLOW_SOMEIPMESSAGEIMPL_H
