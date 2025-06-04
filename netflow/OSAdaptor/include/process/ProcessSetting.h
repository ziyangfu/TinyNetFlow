
#ifndef OSADAPTOR_PROCESS_PROCESS_SETTING_H
#define OSADAPTOR_PROCESS_PROCESS_SETTING_H

#include "process/Types.h"
#include <optional>

namespace osadaptor::process {
/*!
 * \brief CGroup设置，要设置路径、cpu限制、内存限制必须将isSettingCGroup设置为true
 * \details
 *      禁止设置resourceGroupPath_为目录下的 netflow_remove_temp.group，该分组专为删除用
 * */
struct CGroupSettings {
    bool isSettingCGroup = false;
    std::optional<std::string> resourceGroupPath_;
    std::optional<double> cpuLimit_;
    std::optional<std::uint32_t> memoryLimit_;
};

/*!
 * \brief 进程设置：
 *          调度策略、调度优先级为必选项；
 *          如果是rt进程，且需要运行在隔离的CPU核心，则：
 *              1. 设置 isRunInIsolatedCpu_ = true
 *              2. cpuAffinity_必须有值，且设置为0/1, 且若需要运行在1，则必须设置Process::setIsolatedCpuCount(2)
 *          其余选项均为可选项
 * */
struct ProcessSettings {
    bool isRunInIsolatedCpu_ = false;
    SchedulerPolicy schePolicy_ = SchedulerPolicy::RR;
    int schePriority_ = 0;
    std::optional<std::uint32_t> cpuAffinity_;  /** 最多0/1为隔离核心 */
    std::optional<std::uint32_t> userId_;
    std::optional<std::uint32_t> userGroupId_;
    std::optional<std::string> currentWorkDir_;
    CGroupSettings cGroupSettings_;

};

}  // namespace osadaptor::process


#endif //OSADAPTOR_PROCESS_PROCESS_SETTING_H
