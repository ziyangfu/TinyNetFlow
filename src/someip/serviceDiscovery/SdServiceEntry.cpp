//
// Created by fzy on 23-9-12.
//

#include "SdServiceEntry.h"


using namespace netflow::net::someip;

MinorVersion SdServiceEntry::getMinorVersion() const {
    return minorVersion_;
}

void SdServiceEntry::setMinorVersion(netflow::net::someip::MinorVersion version) {
    minorVersion_ = version;
}