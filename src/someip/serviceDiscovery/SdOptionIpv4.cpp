//
// Created by fzy on 23-9-12.
//

#include "SdOptionIpv4.h"

using namespace netflow::net::someip;

SdOptionIpv4::SdOptionIpv4(bool isMulticast, std::string& addr, uint16_t port)
    : SdOption(0x0009),
      addr_(addr),
      port_(port),
      isUdp_(isMulticast)  /** 组播只能UDP */
{
    if (isMulticast) {
        type_ = SomeIpSdOptionType::IP4_MULTICAST;
    }
    else {
        type_ = SomeIpSdOptionType::IP4_ENDPOINT;
    }
}

const std::string SdOptionIpv4::getAddr() const {
    return addr_;
}

void SdOptionIpv4::setAddr(std::string &addr) {
    addr_ = addr;
}

uint16_t SdOptionIpv4::getPort() const {
    return port_;
}

void SdOptionIpv4::setPort(uint16_t port) {
    port_ = port;
}

bool SdOptionIpv4::isUsingUdp() {
    return isUdp_;
}

bool SdOptionIpv4::settingUseUdp(bool on) {
    isUdp_ = on;
}

bool SdOptionIpv4::isMulticast() const {
    return type_ == SomeIpSdOptionType::IP4_MULTICAST;
}
