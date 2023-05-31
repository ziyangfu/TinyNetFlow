//
// Created by fzy on 23-5-25.
//

#ifndef TINYNETFLOW_TIMER_H
#define TINYNETFLOW_TIMER_H

#include <atomic>

#include "Callbacks.h"
#include "../base/Timestamp.h"

using namespace netflow::base;

namespace netflow::net {

/*!
* \brief 定时器 */
class Timer {
public:
    Timer(TimerCallback cb, Timestamp when, double interval)
        : callback_(cb),
          expiration_(when),
          interval_(interval),
          repeat_(interval > 0.0),
          sequence_(0)
          {}
    ~Timer() = default;

    void run() const { callback_(); }
    Timestamp getExpiration() const { return expiration_; }
    bool repeat() const { return repeat_; }
    int64_t sequence() const { return sequence_; }

    void restart(Timestamp now);

private:
    const TimerCallback callback_;
    Timestamp expiration_; /** 失效时间 */
    const double interval_;  /* FIXME： std::chrono::duration */
    const bool repeat_;
    const int64_t sequence_;

    static std::atomic_int64_t s_numCreated_;
};
} // namespace netflow::net



#endif //TINYNETFLOW_TIMER_H
