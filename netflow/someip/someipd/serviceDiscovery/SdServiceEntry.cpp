//
// Created by fzy on 23-9-12.
//

#include "SdServiceEntry.h"


using namespace netflow::net::someip;

MinorVersion SdServiceEntry::getMinorVersion() const {
    return minorVersion_;
}

void SdServiceEntry::setMinorVersion(MinorVersion version) {
    minorVersion_ = version;
}