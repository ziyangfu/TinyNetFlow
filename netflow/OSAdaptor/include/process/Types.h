//
// Created by fzy on 2025/1/24.
//

#ifndef TINYNETFLOW_TYPES_H
#define TINYNETFLOW_TYPES_H

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

/*!
 * \brief 进程设置：调度策略、调度优先级； 可选设置 CPU 亲和性
 * */
struct ProcessSettings {
    SchedulerPolicy policy = SchedulerPolicy::RR;
    int priority = 0;
    std::optional<cpu_set_t> cpuAffinity;
};


}
#endif //TINYNETFLOW_TYPES_H
