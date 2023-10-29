//
// Created by fzy on 23-10-24.
//

#include "ApplicationImpl.h"


using namespace netflow::net::someip;

void ApplicationImpl::init() {

}

void ApplicationImpl::start() {

}


void ApplicationImpl::stop() {

}

void ApplicationImpl::offerService(ServiceId serviceId, InstanceId instanceId,
                              MajorVersion majorVersion,
                              MinorVersion minorVersion, SomeIpTtl ttl) {

}

void ApplicationImpl::stopOfferService(ServiceId serviceId, InstanceId instanceId) {

}

void ApplicationImpl::requestService(ServiceId serviceId, InstanceId instanceId, MajorVersion majorVersion,
                                     MinorVersion minorVersion, SomeIpTtl ttl) {

}

void ApplicationImpl::releaseService(ServiceId serviceId, InstanceId instanceId) {

}

void ApplicationImpl::subscribe(ServiceId serviceId, InstanceId instanceId, EventGroupId eventGroupId,
                                MajorVersion majorVersion, MinorVersion minorVersion, SomeIpTtl ttl) {

}

void ApplicationImpl::unsubscribe(ServiceId serviceId, InstanceId instanceId, EventGroupId eventGroupId) {

}


void ApplicationImpl::notify(ServiceId serviceId, InstanceId instanceId, EventId eventId,
                             std::shared_ptr<Message> message) const {

}

void ApplicationImpl::notifyOne(ServiceId serviceId, InstanceId instanceId, EventId eventId,
                                std::shared_ptr<Message> message, ClientId clientId) const {

}

void ApplicationImpl::send(std::shared_ptr<Message> message, bool flush) {

}

bool ApplicationImpl::isAvailable(ServiceId serviceId, InstanceId instanceId) {

}

void ApplicationImpl::registerAvailableHandler(ServiceId serviceId, InstanceId instanceId,
                                               AvailabilityHandler handler) {

}

void ApplicationImpl::unregisterAvailableHandler(ServiceId serviceId, InstanceId instanceId) {

}

void ApplicationImpl::registerMessageHandler(ServiceId serviceId, InstanceId instanceId, MethodId methodId,
                                             MessageHandler messageHandler) {


}

void ApplicationImpl::unregisterMessageHandler(ServiceId serviceId, InstanceId instanceId, MethodId methodId) {

}

void ApplicationImpl::registerSubscriptionHandler(ServiceId serviceId, InstanceId instanceId,
                                                  EventGroupId eventGroupId,
                                                  SubscriptionHandler handler) {

}

void ApplicationImpl::unregisterSubscriptionHandler(ServiceId serviceId, InstanceId instanceId,
                                               EventGroupId eventGroupId) {

}