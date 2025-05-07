#include "thread/ThisThread.h"
#include <pthread.h>

namespace osadaptor::thread {

template<typename Rep, typename Period>
void thisThread::sleepFor(const std::chrono::duration<Rep, Period> &rtime) {
    if (rtime <= rtime.zero())
        return;
    auto __s = std::chrono::duration_cast<std::chrono::seconds>(rtime);
    auto __ns = std::chrono::duration_cast<std::chrono::nanoseconds>(rtime - __s);
    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(__s.count());
    ts.tv_nsec = static_cast<long>(__ns.count());
    // 使用 nanosleep 实现休眠
    while (::nanosleep(&ts, &ts) == -1 && errno == EINTR) {
        // 如果被信号中断，则继续休眠
    }
}

template<typename Clock, typename Duration>
void thisThread::sleepUtil(const std::chrono::time_point<Clock, Duration> &atime) {
    // 获取当前时间点
    auto now = Clock::now();
    if (Clock::is_steady) {
        if (now < atime) {
            sleepFor(atime - now);
        }
        return;
    }
    while (now < atime) {
        sleepFor(atime - now);
        now = Clock::now();
    }
}

std::string thisThread::getName() {
    std::string threadName{};
    int const ret = pthread_getname_np(pthread_self(), threadName.data(), threadName.size());
    if (ret != 0) {
        threadName = "unknown";
    }
    return threadName;
}

}  // namespace osadaptor::thread {