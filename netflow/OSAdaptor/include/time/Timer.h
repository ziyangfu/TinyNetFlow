//
// Created by fzy on 23-5-25.
//

#ifndef TINYNETFLOW_OSADAPTOR_TIMER_H
#define TINYNETFLOW_OSADAPTOR_TIMER_H

#include <atomic>

#include "netflow/OSAdaptor/include/IO/net/Callbacks.h"
#include "Timestamp.h"

using namespace netflow::base;

namespace netflow::net {

/*!
* \brief 定时器 */
class Timer {
public:
    /** cb : 定时器回调函数
     * when： 到什么时候为止时的时间戳， 触发闹钟
     * interval： 周期性闹钟，间隔时间为多少 */
    Timer(TimerCallback cb, Timestamp when, double interval);
    ~Timer() = default;

    void run() const { callback_(); }
    /** 获取到期时的时间戳 */
    Timestamp getExpiration() const { return expiration_; }
    bool repeat() const { return repeat_; }
    int64_t sequence() const { return sequence_; }

    void restart(Timestamp now);

private:
    const TimerCallback callback_;
    Timestamp expiration_; /** 到期时间的时间戳 */
    const double interval_;  /* FIXME： std::chrono::duration */
    const bool repeat_;
    const int64_t sequence_;
    /** 创建了多少个定时器 */
    static std::atomic_int64_t s_numCreated_;
};
} // namespace netflow::net



#endif //TINYNETFLOW_OSADAPTOR_TIMER_H
