//
// Created by fzy on 23-8-22.
//

#include "SomeIpMessageImpl.h"

using namespace netflow::net::someip;

MessageId SomeIpMessageImpl::getMessageId() const {

}

void SomeIpMessageImpl::setMessageId(MessageId messageId) {

}

ServiceId SomeIpMessageImpl::getServiceId() const {
    return header_.serviceId_;
}

void SomeIpMessageImpl::setServiceId(netflow::net::someip::ServiceId serviceId) {
    header_.serviceId_ = serviceId;
}

MethodId SomeIpMessageImpl::getMethodId() const {
    return header_.methodId_;
}

void SomeIpMessageImpl::setMethodId(netflow::net::someip::MethodId methodId) {
    header_.methodId_ = methodId;
}

InstanceId SomeIpMessageImpl::getInstanceId() const {
    //return
}

void SomeIpMessageImpl::setInstanceId(netflow::net::someip::InstanceId instanceId) {

}