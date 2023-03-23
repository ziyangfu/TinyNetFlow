//
// Created by fzy on 23-3-22.
//

#include "condition.h"
#include <errno.h>

namespace muduo {
    bool Condition::waitForSeconds(double seconds) {
        struct timespec abstime;
        clock_gettime(CLOCK_REALTIME, &abstime);   //! 获取系统时间

        const int64_t kNanoSecondsPerSecond = 1000000000;
        int64_t nanoseconds = static_cast<int64_t>(seconds * kNanoSecondsPerSecond);  //! 超时等待时间为多少纳秒
        //! 系统时间 + 超时等待时间 （秒 + 纳秒 ）
        abstime.tv_sec += static_cast<time_t>((abstime.tv_nsec + nanoseconds) / kNanoSecondsPerSecond );
        abstime.tv_nsec = static_cast<long>((abstime.tv_nsec + nanoseconds) % kNanoSecondsPerSecond));

        MutexLock::UnassignGuard ug(mutex_);
        return ETIMEDOUT == pthread_cond_timedwait(&pcond_, mutex_.getPthreadMutex(), &abstime);
    }
} // muduo