//
// Created by fzy on 23-9-12.
//

#ifndef TINYNETFLOW_SDENTRY_H
#define TINYNETFLOW_SDENTRY_H

#include "netflow/someip/interface/EnumType.h"
#include "netflow/someip/interface/TypeDefine.h"

#include <vector>

namespace netflow::net::someip {
/*!
 * \brief 服务发现entry部分接口类 */
class SdEntry {
public:
    virtual ~SdEntry() = default;

    SomeIpSdEntryType getType() const { return type_; }
    void setType(SomeIpSdEntryType type) { type_ = type; }

    ServiceId getServiceId() const { return serviceId_; }
    void setServiceId(ServiceId id) { serviceId_ = id; }

    InstanceId getInstanceId() const { return instanceId_; }
    void setInstanceId(InstanceId id) { instanceId_ = id; }

    MajorVersion getMajorVersion() { return majorVersion_; }
    void setMajorVersion(MajorVersion version) { majorVersion_ = version; }

    SomeIpTtl getTtl() const { return ttl_; }
    void setTtl(SomeIpTtl ttl) { ttl_ = ttl; }

    bool isServiceEntry() const { return type_ <= SomeIpSdEntryType::REQUEST_SERVICE; }
    bool isEventGroupEntry() const { return (type_ >= SomeIpSdEntryType::FIND_EVENT_GROUP &&
                                             type_ <= SomeIpSdEntryType::SUBSCRIBE_EVENTGROUP_ACK); }

protected:
    SdEntry();
    SdEntry(const SdEntry& entry);
    SdEntry& operator=(const SdEntry& entry) = delete;
    SdEntry(SdEntry&& entry) = delete;
    SdEntry& operator=(SdEntry&& entry) = delete;
protected:
    SomeIpSdEntryType type_;  /** SD entry 的公共部分 */
    ServiceId serviceId_;
    InstanceId instanceId_;
    MajorVersion majorVersion_;
    SomeIpTtl ttl_;

    std::vector<uint8_t> options_[2];
};
}  // namespace netflow::net::someip



#endif //TINYNETFLOW_SDENTRY_H
