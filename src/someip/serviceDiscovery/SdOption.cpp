//
// Created by fzy on 23-9-12.
//

#include "SdOption.h"

using namespace netflow::net::someip;

uint16_t SdOption::getLength() const {
    return length;
}

SomeIpSdOptionType SdOption::getSdOptionType() const {
    return type_;
}