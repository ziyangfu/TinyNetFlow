//
// Created by fzy on 2025/1/24.
//

#ifndef TINYNETFLOW_PROCESS_H
#define TINYNETFLOW_PROCESS_H

#include <string>
#include <vector>
#include "process/Types.h"

namespace osadaptor::process {
/*!
 *  1. set scheduler policy and priority
 *  2. create process
 *
 * */
class Process {
public:
    Process() = default;
    ~Process() = default;

    void processCreate(const std::string& program, const std::vector<std::string>& args);

    void setProcessSettings(ProcessSettings settings);
    void setProcessName(const std::string& name);
    ProcessSettings getProcessSettings();
    /*!
     * \brief 设置CPU亲和性
     * */
    void setCpuAffinity();
    static std::string schedulerPolicyToString(SchedulerPolicy policy);
private:
    bool configureScheduler(pid_t pid, SchedulerPolicy policy, int priority);
    int getSystemCpuCoreCount();
    bool isValidCpuSet(const cpu_set_t& cpuSet, int numCpus) {
        for (int i = 0; i < numCpus; ++i) {
            if (CPU_ISSET(i, &cpuSet)) {
                return true; // 至少一个 CPU 核心被设置
            }
        }
        return false; // 没有 CPU 核心被设置
    }
private:
    ProcessSettings settings_;
};




}


#endif //TINYNETFLOW_PROCESS_H
