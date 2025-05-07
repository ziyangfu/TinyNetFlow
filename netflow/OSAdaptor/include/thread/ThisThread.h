/**
 * \brief 实现 std::thread 的部分功能，实际是抄的部分 std::thread 的实现。
 * */

#ifndef OSADAPTOR_THREAD_THIS_THREAD_H
#define OSADAPTOR_THREAD_THIS_THREAD_H

#include <chrono>
#include <string>

namespace osadaptor::thread {
namespace thisThread {

template<typename Rep, typename Period>
void sleepFor(const std::chrono::duration<Rep, Period> &rtime);

template<typename Clock, typename Duration>
void sleepUtil(const std::chrono::time_point<Clock, Duration> &atime);

std::string getName();

} // namespace thisThread
} // namespace osadaptor::thread

#endif //OSADAPTOR_THREAD_THIS_THREAD_H
