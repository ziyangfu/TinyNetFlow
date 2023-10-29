//
// Created by fzy on 23-9-12.
//

#ifndef TINYNETFLOW_SDEVENTGROUPENTRY_H
#define TINYNETFLOW_SDEVENTGROUPENTRY_H

#include "netflow/someip/someipd/serviceDiscovery/SdEntry.h"

namespace netflow::net::someip {

class SdEventGroupEntry : public SdEntry {
public:
    SdEventGroupEntry()
        : eventGroupId_(0)
    {}
    ~SdEventGroupEntry();

    EventGroupId getEventGroupId() const;
    void setEventGroupId(EventGroupId id);
private:
    EventGroupId eventGroupId_; /** SD event group 独有部分 */
};

}  // namespace netflow::net::someip



#endif //TINYNETFLOW_SDEVENTGROUPENTRY_H
