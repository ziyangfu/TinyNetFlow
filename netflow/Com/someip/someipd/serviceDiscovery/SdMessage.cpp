//
// Created by fzy on 23-8-28.
//

#include "SdMessage.h"
#include "SdServiceEntry.h"
#include "SdEventGroupEntry.h"
#include "SdOptionIpv4.h"
#include "SdOptionIpv6.h"
#include "netflow/Com/someip/interface/Constants.h"


using namespace netflow::net::someip;

SdMessage::SdMessage()
    : flags_(SomeIpSdFlag::NON_ALL),
      reserved_({0,0,0}),
      entriesLength(0),
      entries_(),
      optionsLength(0),
      options_()
{
    header_.serviceId_      = kSdServiceId;
    header_.methodId_       = kSdMethodId;
    header_.clientId_       = kSdClientId;
    header_.instanceId_     = kSdInstanceId;
    /** header_.protocolVersion_ 已经在header_的构造函数中设置 */
    header_.messageType_    = SomeIpMessageType::MT_NOTIFICATION;
    header_.returnCode_     = SomeIpReturnCode::E_OK;
}

SomeIpLength SdMessage::getLength() const {

}

void SdMessage::setLength(SomeIpLength len) {
    header_.length_ = len;
}

SomeIpSdFlag SdMessage::getFlag() const {
    return flags_;
}

void SdMessage::setFlag(SomeIpSdFlag flag) {
    flags_ = flag;
}

std::shared_ptr<SdServiceEntry> SdMessage::createServiceEntry() {
    std::shared_ptr<SdServiceEntry> entry = std::make_shared<SdServiceEntry>();
    entries_.push_back(entry);
    return entry;
}

std::shared_ptr<SdEventGroupEntry> SdMessage::createEventGroupEntry() {
    std::shared_ptr<SdEventGroupEntry> entry = std::make_shared<SdEventGroupEntry>();
    entries_.push_back(entry);
    return entry;
}


SdOptionIpv4 SdMessage::createOptionIpv4() {

}

SdOptionIpv6 SdMessage::createOptionIpv6() {

}