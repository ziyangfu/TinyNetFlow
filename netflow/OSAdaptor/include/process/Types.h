
#ifndef OSADAPTOR_PROCESS_TYPES_H
#define OSADAPTOR_PROCESS_TYPES_H

#include <sched.h>
#include <optional>

namespace osadaptor::process {

/*!
 * \brief 定义调度策略和优先级
 * \details
 *      RR/FIFO 实时调度策略
 *          优先级由低到高为： 1 ~ 99
 *      OTHER、BATCHD等非实时调度策略
 *          优先级为动态优先级，即nice值， 范围为 -20 ~ 19
 *              -20 是最高优先级
 *              19 是最低优先级
 *              默认优先级是 0
 *      SCHED_DEADLINE 调度策略（每个任务在指定的时间段内必须完成其工作量）
 *              静态优先级， 优先级是由 runtime、period 和 deadline 参数决定的，而不是通过传统的优先级数值来表示
 * */
enum class SchedulerPolicy {
    OTHER = SCHED_OTHER, /** Linux默认， CFS 完全公平调度器， 非实时调度器 */
    FIFO = SCHED_FIFO,
    RR = SCHED_RR,       /** 本中间件默认， 时间片轮转调度，实时调度策略 */
    BATCH = SCHED_BATCH, /** 类似OTHER, 减少了抢占频率，适用于计算密集型任务如批处理任务，减少上下文切换开销 */
    IDLE = SCHED_IDLE,
    DEADLINE = SCHED_DEADLINE  /** 不用 */
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


}
#endif //OSADAPTOR_PROCESS_TYPES_H
