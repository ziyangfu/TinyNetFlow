//
// Created by fzy on 23-3-13.
//

/*! 定时器回调 */

#ifndef LIBZV_TIMER_H
#define LIBZV_TIMER_H

#include "../base/atomic.h"
#include "../base/timestamp.h"
#include "callback.h"

namespace muduo {
namespace net {
//! 内部使用的定时器类
class Timer : Noncopyable {
public:
    Timer(TimerCallback cb, Timestamp when, double interval)
        : callback_(cb),
        expiration_(when),
        interval_(interval),
        repeat_(interval > 0.0),
        sequence_(s_numCreated_.incrementAndGet())
    {}

    void run() const { callback_(); }

    Timestamp expiration() const { return expiration_; }

    bool repeat() const { return repeat_; }

    int64_t sequence() const { return sequence_; }

    static int64_t numCreated() { return s_numCreated_.get(); }

    void restart(Timestamp now);


private:
    const TimerCallback callback_;
    Timestamp expiration_; //! 终止
    const double interval_;
    const bool repeat_;
    const int64_t sequence_;

    static AtomicInt64 s_numCreated_;

};
} // namespace net

} // namespace muduo

#endif //LIBZV_TIMER_H
