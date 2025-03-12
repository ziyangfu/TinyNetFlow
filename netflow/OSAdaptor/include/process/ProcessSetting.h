
#ifndef OSADAPTOR_PROCESS_PROCESS_SETTING_H
#define OSADAPTOR_PROCESS_PROCESS_SETTING_H

#include "process/Types.h"
#include <optional>

namespace osadaptor::process {

/*!
 * \brief 进程设置：
 *          调度策略、调度优先级为必选项；
 *          其余选项均为可选项
 * */
struct ProcessSettings {
    SchedulerPolicy schePolicy_ = SchedulerPolicy::RR;
    int schePriority_ = 0;
    std::optional<cpu_set_t> cpuAffinity_;
    std::optional<uid_t> userId_;
    std::optional<gid_t> userGroupId_;
    std::optional<std::string> resourceGroupPath_;
};

}  // namespace osadaptor::process


#endif //OSADAPTOR_PROCESS_PROCESS_SETTING_H
