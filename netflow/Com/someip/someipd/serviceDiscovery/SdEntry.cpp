//
// Created by fzy on 23-9-12.
//

#include "SdEntry.h"

using namespace netflow::net::someip;

SdEntry::SdEntry()
    : type_(SomeIpSdEntryType::UNKNOWN),
      serviceId_(0x0),
      instanceId_(0x0),
      majorVersion_(0),
      ttl_(0x0)
{
}

SdEntry::SdEntry(const netflow::net::someip::SdEntry &entry) {
    type_ = entry.type_;
    serviceId_ = entry.serviceId_;
    instanceId_ = entry.instanceId_;
    majorVersion_ = entry.majorVersion_;
    ttl_ = entry.ttl_;
}