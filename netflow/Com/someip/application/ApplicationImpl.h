//
// Created by fzy on 23-10-24.
//

#ifndef TINYNETFLOW_APPLICATIONIMPL_H
#define TINYNETFLOW_APPLICATIONIMPL_H

#include "netflow/Com/someip/interface/Application.h"
#include <mutex>
#include <string>
#include <memory>
#include <map>
#include <set>

/** application 实现类 */
namespace netflow::net::someip {

class Configuration;  /** 从json文件中解析出配置信息 */

class ApplicationImpl : public Application {
public:
    ApplicationImpl();
    ~ApplicationImpl();

    void init() override;
    void start() override;
    void stop() override;

    void offerService(ServiceId serviceId,
                      InstanceId instanceId,
                      MajorVersion majorVersion,
                      MinorVersion minorVersion,
                      SomeIpTtl ttl) override;
    void stopOfferService(ServiceId serviceId, InstanceId instanceId) override;

    void requestService(ServiceId serviceId,
                        InstanceId instanceId,
                        MajorVersion majorVersion,
                        MinorVersion minorVersion,
                        SomeIpTtl ttl) override;
    void releaseService(ServiceId serviceId, InstanceId instanceId) override;

    void subscribe(ServiceId serviceId,
                  InstanceId instanceId,
                  EventGroupId eventGroupId,
                  MajorVersion majorVersion,
                  MinorVersion minorVersion,
                  SomeIpTtl ttl) override;
    void unsubscribe(ServiceId serviceId, InstanceId instanceId, EventGroupId eventGroupId) override;

    void notify(ServiceId serviceId,
               InstanceId instanceId,
               EventId eventId,
               std::shared_ptr<Message> message) const override;

    void notifyOne(ServiceId serviceId,
                  InstanceId instanceId,
                  EventId eventId,
                  std::shared_ptr<Message> message,
                  ClientId clientId) const override;

    void send(std::shared_ptr<Message> message, bool flush = true) override;
    bool isAvailable(ServiceId serviceId, InstanceId instanceId) override;
    void registerAvailableHandler(ServiceId serviceId,
                                  InstanceId instanceId,
                                  AvailabilityHandler handler) override;
    void unregisterAvailableHandler(ServiceId serviceId, InstanceId instanceId) override;

    void registerMessageHandler(ServiceId serviceId,
                                InstanceId instanceId,
                                MethodId methodId,
                                MessageHandler messageHandler) override;
    void unregisterMessageHandler(ServiceId serviceId, InstanceId instanceId, MethodId methodId) override;

    void registerSubscriptionHandler(ServiceId serviceId,
                                     InstanceId instanceId,
                                     EventGroupId eventGroupId,
                                     SubscriptionHandler handler) override;
    void unregisterSubscriptionHandler(ServiceId serviceId,
                                       InstanceId instanceId,
                                       EventGroupId eventGroupId) override;

    /** SOME/IP的三种方法
     *      method
     *      event
     *      field */
private:
    ClientId clientId_;
    SessionId sessionId_;
    std::string name_;
    std::shared_ptr<Configuration> configuration_;

    /** 保存消息回调函数 */
    std::map<ServiceId, std::map<InstanceId, std::map<MethodId, MessageHandler>>> members_;
    mutable std::mutex membersMutex_;

    /** 保存连接回调函数 */
    std::map<ServiceId, std::map<InstanceId, AvailabilityHandler>> availability_;
    /** 一个服务可能有多个实例 */
    mutable std::map<ServiceId, std::set<InstanceId>> available_;

    /** 记录一个订阅事件， 保存订阅事件回调函数 */
    std::map<ServiceId, std::map<InstanceId, std::map<EventGroupId, SubscriptionHandler>>> subscription_;

    /** 信号处理，新开线程处理？ */

    /** 线程池相关 */
    std::size_t numThreads_;
};

}  // namespace netflow::net::someip
#endif //TINYNETFLOW_APPLICATIONIMPL_H
