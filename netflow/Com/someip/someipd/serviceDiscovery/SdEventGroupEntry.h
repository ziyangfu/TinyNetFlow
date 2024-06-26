//
// Created by fzy on 23-9-12.
//

#ifndef TINYNETFLOW_SDEVENTGROUPENTRY_H
#define TINYNETFLOW_SDEVENTGROUPENTRY_H

#include "SdEntry.h"

namespace netflow::net::someip {

class SdEventGroupEntry : public SdEntry {
public:
    SdEventGroupEntry()
        : eventGroupId_(0),
          count_(0)
    {}
    ~SdEventGroupEntry();

    EventGroupId getEventGroupId() const;
    void setEventGroupId(EventGroupId id);
private:
    EventGroupId eventGroupId_; /** SD event group 独有部分 */
    std::uint8_t count_;
};

}  // namespace netflow::net::someip



#endif //TINYNETFLOW_SDEVENTGROUPENTRY_H
