//
// Created by fzy on 23-9-12.
//

#include "SdOptionIpv6.h"

using namespace netflow::net::someip;

SdOptionIpv6::SdOptionIpv6(bool isMulticast, std::string& addr6, uint16_t port)
    : SdOption(0x0015U),
      addr6_(addr6),
      port_(port),
      isUdp_(isMulticast)
{
    if (isMulticast) {
        type_ = SomeIpSdOptionType::IP6_MULTICAST;
    }
    else {
        type_ = SomeIpSdOptionType::IP6_ENDPOINT;
    }
}

const std::string SdOptionIpv6::getAddr6() const {
    return addr6_;
}

void SdOptionIpv6::setAddr6(std::string &addr6) {
    addr6_ = addr6;
}

uint16_t SdOptionIpv6::getPort() const {
    return port_;
}

void SdOptionIpv6::setPort(uint16_t port) {
    port_ = port;
}

bool SdOptionIpv6::isUsingUdp() {
    return isUdp_;
}

bool SdOptionIpv6::settingUseUdp(bool on) {
    isUdp_ = on;
}

bool SdOptionIpv6::isMulticast() const {
    return type_ == SomeIpSdOptionType::IP6_MULTICAST;
}
