/**
 * \brief 实现 std::thread 的部分功能，实际是抄的部分 std::thread 的实现。
 * */

#ifndef OSADAPTOR_THREAD_THIS_THREAD_H
#define OSADAPTOR_THREAD_THIS_THREAD_H

#include <chrono>
#include <string>

namespace osadaptor::thread {
namespace thisThread {
/*!
\details
    C++ 模板不能像普通函数一样分开声明和实现。否则会在链接时出错
    生成最终可执行程序时就会出现错误(在链接时会出错)。因为在编译时模板并不能生成真正的二进制代码，
    而是在编译调用模板类或函数的CPP文件时才会去找对应的模板声明和实现，
    在这种情况下编译器是不知道实现模板类或函数的CPP文件的存在，所以它只能找到模板类或函数的声明而找不到实现，
    而只好创建一个符号寄希望于链接程序找地址。但模板类或函数的实现并不能被编译成二进制代码，
    结果链接程序找不到地址只好报错了。
    更多查看：[原理：C++为什么一般把模板实现放入头文件](https://www.cnblogs.com/zpcdbky/p/16329886.html)
 */
template<typename Rep, typename Period>
void sleepFor(const std::chrono::duration<Rep, Period> &rtime) {
    if (rtime <= rtime.zero())
        return;
    auto _s = std::chrono::duration_cast<std::chrono::seconds>(rtime);
    auto _ns = std::chrono::duration_cast<std::chrono::nanoseconds>(rtime - _s);
    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(_s.count());
    ts.tv_nsec = static_cast<long>(_ns.count());
    // 使用 nanosleep 实现休眠
    while (::nanosleep(&ts, &ts) == -1 && errno == EINTR) {
        // 如果被信号中断，则继续休眠
    }
}

template<typename Clock, typename Duration>
void sleepUtil(const std::chrono::time_point<Clock, Duration> &atime) {
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

std::string getName();

} // namespace thisThread
} // namespace osadaptor::thread

#endif //OSADAPTOR_THREAD_THIS_THREAD_H
