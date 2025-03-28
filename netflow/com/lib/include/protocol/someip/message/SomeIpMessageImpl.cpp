//
// Created by fzy on 23-8-22.
//

#include "SomeIpMessageImpl.h"
#include "netflow/Com/someip/interface/Runtime.h"

using namespace netflow::net::someip;

SomeIpMessageImpl::SomeIpMessageImpl()
    : header_(),
      payloadPtr(Runtime::get()->createPayload())  /** 基类指针指向子类对象 */
{
}

SomeIpMessageImpl::~SomeIpMessageImpl() noexcept {

}


const SomeIpHeader &SomeIpMessageImpl::getHeader() const {
    return header_;
}

std::shared_ptr<Payload> SomeIpMessageImpl::getPayload() const {
    return payloadPtr;
}

void SomeIpMessageImpl::setPayload(std::shared_ptr<Payload> payload) {
    payloadPtr = payload;
}

MessageId SomeIpMessageImpl::getMessageId() const {
    return header_.messageId_;
}

void SomeIpMessageImpl::setMessageId(MessageId messageId) {
    header_.messageId_ = messageId;
}

RequestId SomeIpMessageImpl::getRequestId() const {
    return header_.requestId_;
}

void SomeIpMessageImpl::setRequestId(RequestId requestId) {
    header_.requestId_ = requestId;
}

InstanceId SomeIpMessageImpl::getInstanceId() const {
    return header_.instanceId_;
}

void SomeIpMessageImpl::setInstanceId(InstanceId instanceId) {
    header_.instanceId_ = instanceId;
}
/** ------------------------SOME/IP Header ---------------------------------*/
ServiceId SomeIpMessageImpl::getServiceId() const {
    return header_.serviceId_;
}

void SomeIpMessageImpl::setServiceId(ServiceId serviceId) {
    header_.serviceId_ = serviceId;
}

MethodId SomeIpMessageImpl::getMethodId() const {
    return header_.methodId_;
}

void SomeIpMessageImpl::setMethodId(MethodId methodId) {
    header_.methodId_ = methodId;
}

SomeIpLength SomeIpMessageImpl::getLength() const {
    return header_.length_;
}

void SomeIpMessageImpl::setLength(SomeIpLength length) {
    header_.length_ = length;
}

ClientId SomeIpMessageImpl::getClientId() const {
    return header_.clientId_;
}

void SomeIpMessageImpl::setClientId(ClientId clientId) {
    header_.clientId_ = clientId;
}

ProtocolVersion SomeIpMessageImpl::getProtocolVersion() const {
    return header_.protocolVersion_;
}

void SomeIpMessageImpl::setProtocolVersion(ProtocolVersion protocolVersion) {
    header_.protocolVersion_ = protocolVersion;
}

InterfaceVersion SomeIpMessageImpl::getInterfaceVersion() const {
    return header_.interfaceVersion_;
}

void SomeIpMessageImpl::setInterfaceVersion(InterfaceVersion interfaceVersion) {
    header_.interfaceVersion_ = interfaceVersion;
}

SomeIpMessageType SomeIpMessageImpl::getMessageType() const {
    return header_.messageType_;
}

void SomeIpMessageImpl::setMessageType(SomeIpMessageType type) {
    header_.messageType_ = type;
}

SomeIpReturnCode SomeIpMessageImpl::getReturnCode() const {
    return header_.returnCode_;
}

void SomeIpMessageImpl::setReturnCode(SomeIpReturnCode returnCode) {
    header_.returnCode_ = returnCode;
}
/** ------------------------ end of SOME/IP Header ---------------------------------*/
