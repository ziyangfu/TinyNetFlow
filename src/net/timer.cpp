//
// Created by fzy on 23-3-13.
//

#include "timer.h"

using namespace muduo;
using namespace muduo::net;

AtomicInt64 Timer::s_numCreated_;   //! 静态变量类外声明
void Timer::restart(Timestamp now) {
    if (repeat_) {
        expiration_ = addTime(now, interval_);
    }
    else {
        expiration_ = Timestamp::invalid();
    }
}