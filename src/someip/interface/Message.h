//
// Created by fzy on 23-9-10.
//

#ifndef TINYNETFLOW_MESSAGE_H
#define TINYNETFLOW_MESSAGE_H

#include <memory>

namespace netflow::net::someip {

class Payload;

class Message {
public:
    Message() = default;
    virtual ~Message() = default;

    virtual std::shared_ptr<Payload> getPayload() const = 0;
    virtual void setPayload(std::shared_ptr<Payload> payload) = 0;

    /** MessageBase */
    virtual uint32_t getMessageId() const = 0;
    virtual void setMessageId(uint32_t messageId) = 0;

    virtual uint16_t getServiceId() const = 0;
    virtual void setServiceId(uint16_t serviceId) = 0;

    virtual uint16_t getInstanceId() const = 0;
    virtual void setInstanceId(uint16_t instanceId) = 0;

    virtual uint16_t getMethodId() const = 0;
    virtual void setMethodId(uint16_t methodId) = 0;

    virtual uint16_t getSessionId() const = 0;
    virtual  void setSessionId(uint16_t sessionId) = 0;

};
}  // namespace netflow::net::someip

#endif //TINYNETFLOW_MESSAGE_H
