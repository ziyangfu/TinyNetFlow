//
// Created by fzy on 23-9-10.
//

#ifndef TINYNETFLOW_APPLICATION_H
#define TINYNETFLOW_APPLICATION_H

#include "TypeDefine.h"
#include "Constants.h"
#include "Message.h"
#include "Handler.h"

#include <memory>

namespace netflow::net::someip {

class Application {
    virtual ~Application() {}

    virtual void init() = 0;
    virtual void start() = 0;
    virtual void stop() = 0;

    virtual void offerService(ServiceId serviceId, InstanceId instanceId, MajorVersion majorVersion,
                              MinorVersion minorVersion, SomeIpTtl ttl) = 0;

    virtual void stopOfferService(ServiceId serviceId, InstanceId instanceId) = 0;

    virtual void requestService(ServiceId serviceId, InstanceId instanceId, MajorVersion majorVersion,
                                MinorVersion minorVersion, SomeIpTtl ttl) = 0;
    virtual void releaseService(ServiceId serviceId, InstanceId instanceId) = 0;

    virtual void subscribe(ServiceId serviceId, InstanceId instanceId, EventGroupId eventGroupId,
                                MajorVersion majorVersion, MinorVersion minorVersion, SomeIpTtl ttl) = 0;
    virtual void unsubscribe(ServiceId serviceId, InstanceId instanceId, EventGroupId eventGroupId) = 0;

    virtual void notify(ServiceId serviceId, InstanceId instanceId, EventId eventId,
                        std::shared_ptr<Message> message) const = 0;
    /** 具体通知那个client ID */
    virtual void notifyOne(ServiceId serviceId, InstanceId instanceId, EventId eventId,
                           std::shared_ptr<Message> message, ClientId clientId) const = 0;

    virtual void send(std::shared_ptr<Message> message, bool flush = true) = 0;

    virtual bool isAvailable(ServiceId serviceId, InstanceId instanceId) = 0;
    virtual void registerAvailableHandler(ServiceId serviceId, InstanceId instanceId,
                                          AvailabilityHandler handler) = 0;
    virtual void unregisterAvailableHandler(ServiceId serviceId, InstanceId instanceId) = 0;

    virtual void registerSubscriptionHandler(ServiceId serviceId, InstanceId instanceId,
                                             EventGroupId eventGroupId, SubscriptionHandler handler) = 0;
    virtual void unregisterSubscriptionHandler(ServiceId serviceId, InstanceId instanceId,
                                               EventGroupId eventGroupId) = 0;

    virtual void registerMessageHandler(ServiceId serviceId, InstanceId instanceId, MethodId methodId,
                                        MessageHandler messageHandler) = 0;
    virtual void unregisterMessageHandler(ServiceId serviceId, InstanceId instanceId, MethodId methodId) = 0;

    /** TODO: eventHandler */



};
}  // namespace netflow::net::someip

#endif //TINYNETFLOW_APPLICATION_H
