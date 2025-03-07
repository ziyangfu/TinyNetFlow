//
// Created by fzy on 2025/1/24.
//


/**
 * 1. 进程创建-程序加载器
 *      1. 程序名称
 *      2. 程序参数
 *      3. 程序执行文件夹
 *      4. 环境变量
 * 2. 进程调度
 *      1. 调度策略与优先级设置
 *      2. CPU 亲和性设置（可选）
 *      2. 时间触发调度器（RT, option）
 * 3. 进程资源管理
 *      1. cgroups 限制进程资源
 *              https://tech.meituan.com/2015/03/31/cgroups.html
 *
 * */

#ifndef OSADAPTOR_PROCESS_PROCESS_H
#define OSADAPTOR_PROCESS_PROCESS_H

#include <string>
#include <vector>
#include "process/Types.h"

namespace osadaptor::process {
class Process {
public:
    Process() = default;
    ~Process() = default;

    void processCreate(const std::string& program, const std::vector<std::string>& args);
    void setProcessSettings(ProcessSettings settings);
    void setProcessName(const std::string& name);
    ProcessSettings getProcessSettings();
    pid_t getPid() const;
    void sendSIGKILL();
    void sendSIGTERM();
    void waitPid();
    void processSync();
    /*!
     * \brief 设置CPU亲和性
     * */
    void setCpuAffinity();
    static std::string schedulerPolicyToString(SchedulerPolicy policy);
private:
    bool configureScheduler(pid_t pid, SchedulerPolicy policy, int priority);
    static int getSystemCpuCoreCount();
    static bool isValidCpuSet(const cpu_set_t& cpuSet, int numCpus);
private:
    ProcessSettings settings_;
};

}


#endif //OSADAPTOR_PROCESS_PROCESS_H
