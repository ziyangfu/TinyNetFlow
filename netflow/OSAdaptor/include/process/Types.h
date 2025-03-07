//
// Created by fzy on 2025/1/24.
//

#ifndef OSADAPTOR_PROCESS_TYPES_H
#define OSADAPTOR_PROCESS_TYPES_H

#include <sched.h>
#include <optional>

namespace osadaptor::process {

/*!
 * \brief 定义调度策略和优先级
 * */
enum class SchedulerPolicy {
    OTHER = SCHED_OTHER,
    FIFO = SCHED_FIFO,
    RR = SCHED_RR,
    BATCH = SCHED_BATCH,
    IDLE = SCHED_IDLE,
    DEADLINE = SCHED_DEADLINE
};

enum class ProcessStatus : std::uint8_t {
    RUNNING,
    STOPPED,
    EXITED,
    ZOMBIE
};

enum class ProcessError : std::uint8_t {
    SUCCESS,
    FAILURE,
    NOT_FOUND,
    ALREADY_EXISTS,
    INVALID_ARGUMENT,
    PERMISSION_DENIED,
    OUT_OF_MEMORY,
    OUT_OF_RESOURCES,
    UNKNOWN
};


enum class TerminationOption : std::uint8_t {
    kBlocking,
    kNonBlocking
};

/*!
 * \brief 进程设置：调度策略、调度优先级； 可选设置 CPU 亲和性
 * */
struct ProcessSettings {
    SchedulerPolicy policy = SchedulerPolicy::RR;
    int priority = 0;
    std::optional<cpu_set_t> cpuAffinity;
};


}
#endif //OSADAPTOR_PROCESS_TYPES_H
