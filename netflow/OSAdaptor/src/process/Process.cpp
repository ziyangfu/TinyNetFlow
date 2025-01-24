//
// Created by fzy on 2025/1/24.
//
#include "process/Process.h"
#include <cerrno>
#include <unistd.h> // 添加头文件以使用 sysconf 函数
#include "spdlog/spdlog.h"

namespace osadaptor::process {

void Process::processCreate(const std::string &program, const std::vector<std::string> &args) {
    std::vector<char*> c_args;
    c_args.push_back(const_cast<char*>(program.c_str()));
    for (const auto& arg : args) {
        c_args.push_back(const_cast<char*>(arg.c_str()));
    }
    c_args.push_back(nullptr);

    execvp(c_args[0], c_args.data());
    SPDLOG_ERROR("Error in process create");
    exit(EXIT_FAILURE);

}

void Process::setCpuAffinity() {
    if (settings_.cpuAffinity.has_value()) {
        if (isValidCpuSet(settings_.cpuAffinity.value(), getSystemCpuCoreCount()) ){
            SPDLOG_ERROR("Invalid CPU affinity value");
            return;
        }
        if (sched_setaffinity(0, sizeof(cpu_set_t), &settings_.cpuAffinity.value()) == -1) {
            SPDLOG_ERROR("Error setting CPU affinity");
            return;
        }
    }
    else {
        SPDLOG_ERROR("No CPU affinity specified, please give a value of CPU affinity");
    }

}

/* static */ std::string Process::schedulerPolicyToString(osadaptor::process::SchedulerPolicy policy) {
    switch (policy) {
        case SchedulerPolicy::OTHER:
            return "SCHED_OTHER";
        case SchedulerPolicy::FIFO:
            return "SCHED_FIFO";
        case SchedulerPolicy::RR:
            return "SCHED_RR";
        case SchedulerPolicy::BATCH:
            return "SCHED_BATCH";
        case SchedulerPolicy::IDLE:
            return "SCHED_IDLE";
        case SchedulerPolicy::DEADLINE:
            return "SCHED_DEADLINE";
        default:
            return "UNKNOWN";
    }
}

bool Process::configureScheduler(pid_t pid, osadaptor::process::SchedulerPolicy policy, int priority) {
    struct sched_param param;
    param.sched_priority = priority;

    int policy_int = static_cast<int>(policy);
    if (sched_setscheduler(pid, policy_int, &param) == -1) {
        SPDLOG_ERROR("Error setting scheduler policy");
        return false;
    }

    if (setpriority(PRIO_PROCESS, pid, priority) == -1) {
        SPDLOG_ERROR("Error setting process priority");
        return false;
    }
    SPDLOG_TRACE("Scheduler policy set to {} with priority {}", schedulerPolicyToString(policy), priority);
    return true;
}

int Process::getSystemCpuCoreCount()
{
    // 使用 sysconf 获取 CPU 核心数
    long num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    if (num_cores == -1) {
        // 处理错误情况，例如返回默认值或抛出异常
        return 1; // 假设至少有一个核心
    }
    return static_cast<int>(num_cores);
}

}  // namespace osadaptor::process