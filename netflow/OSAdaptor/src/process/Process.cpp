//
// Created by fzy on 2025/1/24.
//
#include "process/Process.h"
#include <unistd.h> // 添加头文件以使用 sysconf 函数
#include <sched.h>  // 添加头文件以使用 sched_setscheduler 和 sched_param
#include <sys/resource.h> // 添加头文件以使用 setpriority 和 PRIO_PROCESS
#include <sys/types.h> // 添加头文件以使用 pid_t
#include <sys/wait.h>
#include <vector>
#include "spdlog/spdlog.h"

namespace osadaptor::process {
/*!
 * \brief 进程创建
 * \details
    // 通过fork系统调用创建子进程
    pid{::fork()};
    // 父进程
    if (pid > 0) {
      // 父进程啥也不干
    }
    // 子进程
    else {
      // cgroup资源设置，cgroups
      // 将pid写cgroup资源组
      cgroup.AddPid(pid);
      // 调度策略及优先级设置，底层通过 sched_setscheduler系统调用
      SetSchedulerSettings();
      // 修改工作目录
      chdir();
      // 底层通过 sched_setaffinity 设置CPU亲和性
      sched_setaffinity();
      // 底层通过setgroups 设置补充组ID
      setgroups();
      //底层通过setregid 设置真实有效的组ID
      setregid(gid, gid);
      //底层通过setreuid 设置真实有效的用户ID
      setreuid(uid, uid);
      //底层通过execve，将子进程替换为全新非父子关系的独立进程
      execve();
 * */
void Process::processCreate(const std::string &program, const std::vector<std::string> &args) {
    std::vector<char*> c_args;
    c_args.push_back(const_cast<char*>(program.c_str()));
    for (const auto& arg : args) {
        c_args.push_back(const_cast<char*>(arg.c_str()));
    }
    c_args.push_back(nullptr);

    pid_t pid = ::fork();
    if (pid == -1) {
        SPDLOG_ERROR("Fork failed");
        exit(EXIT_FAILURE);
    }

    // 父进程
    if (pid > 0) {
        // 父进程啥也不干
        return;
    }
    // 子进程
    else {
        // cgroup资源设置，cgroups
        // 假设 cgroup 对象已经初始化
//        Cgroup cgroup("cpu", "my_cgroup");
//        if (!cgroup.create()) {
//            SPDLOG_ERROR("Failed to create cgroup");
//            exit(EXIT_FAILURE);
//        }
//        if (!cgroup.addProcess(getpid())) {
//            SPDLOG_ERROR("Failed to add process to cgroup");
//            exit(EXIT_FAILURE);
//        }

        // 调度策略及优先级设置，底层通过 sched_setscheduler系统调用
        if (!configureScheduler(getpid(), settings_.policy, settings_.priority)) {
            SPDLOG_ERROR("Failed to configure scheduler");
            exit(EXIT_FAILURE);
        }

        // 修改工作目录
        if (chdir("/") == -1) {
            SPDLOG_ERROR("Failed to change directory");
            exit(EXIT_FAILURE);
        }

        // 底层通过 sched_setaffinity 设置CPU亲和性
        setCpuAffinity();

//        // 底层通过setgroups 设置补充组ID
//        if (settings_.supplementaryGroups.has_value()) {
//            if (setgroups(settings_.supplementaryGroups.value().size(), settings_.supplementaryGroups.value().data()) == -1) {
//                SPDLOG_ERROR("Failed to set supplementary groups: {}", strerror(errno));
//                exit(EXIT_FAILURE);
//            }
//        }
//
//        // 底层通过setregid 设置真实有效的组ID
//        if (settings_.gid.has_value()) {
//            if (setregid(settings_.gid.value(), settings_.gid.value()) == -1) {
//                SPDLOG_ERROR("Failed to set real and effective group ID: {}", strerror(errno));
//                exit(EXIT_FAILURE);
//            }
//        }
//
//        // 底层通过setreuid 设置真实有效的用户ID
//        if (settings_.uid.has_value()) {
//            if (setreuid(settings_.uid.value(), settings_.uid.value()) == -1) {
//                SPDLOG_ERROR("Failed to set real and effective user ID: {}", strerror(errno));
//                exit(EXIT_FAILURE);
//            }
//        }

        // 底层通过execve，将子进程替换为全新非父子关系的独立进程
        if (execve(c_args[0], c_args.data(), nullptr) == -1) {
            SPDLOG_ERROR("Execve failed");
            exit(EXIT_FAILURE);
        }
    }

}

ProcessSettings Process::getProcessSettings() {
    return settings_;
}

pid_t Process::getPid() const {
    return getpid();
}

void Process::processSync() {

}

void Process::waitPid() {

}

void Process::sendSIGKILL() {

}


void Process::sendSIGTERM() {

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

/** static */ int Process::getSystemCpuCoreCount()
{
    // 使用 sysconf 获取 CPU 核心数
    long num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    if (num_cores == -1) {
        // 处理错误情况，例如返回默认值或抛出异常
        return 1; // 假设至少有一个核心
    }
    return static_cast<int>(num_cores);
}

/** static */ bool Process::isValidCpuSet(const cpu_set_t &cpuSet, int numCpus) {
    for (int i = 0; i < numCpus; ++i) {
        if (CPU_ISSET(i, &cpuSet)) {
            return true; // 至少一个 CPU 核心被设置
        }
    }
    return false; // 没有 CPU 核心被设置
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

}  // namespace osadaptor::process