//
// Created by fzy on 23-9-12.
//

#include "SdEventGroupEntry.h"
using namespace netflow::net::someip;

EventGroupId SdEventGroupEntry::getEventGroupId() const {
    return eventGroupId_;
}

void SdEventGroupEntry::setEventGroupId(EventGroupId id) {
    eventGroupId_ = id;
}
