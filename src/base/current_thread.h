//
// Created by fzy on 23-3-20.
//

//! 后续可以考虑用 std::thread 替代，虽然用 std::thread无法在top中看到tid的一致

#ifndef LIBZV_CURRENT_THREAD_H
#define LIBZV_CURRENT_THREAD_H

#include "types.h"

namespace muduo {
namespace CurrentThread {
    /*! 见书96页： __thread是GCC内置的线程局部存储设施。
     * __thread使用规则:
     * 1. 无法自动调用构造与析构函数
     * 2. 可以用于修饰全局变量，函数内的静态变量，不能用于修饰函数的局部变量或class的普通成员变量
     * 3. 变量初始化只能用编译器常量
     * 用途：
     * 1. __thread变量是每个线程都有的一份独立实体，各个线程的变量值互不干扰。
     * 2. 修饰那些 “值可能会变，带有全局性，但又不值得用全局锁保护的变量” */
    extern __thread int t_cachedTid;
    extern __thread char t_tidString[32];
    extern __thread int t_tidStringLength;
    extern __thread const char* t_threadName;
    void cacheTid();

    /*! __builtin_expect：GCC内建函数
     * 引入__builtin_expect函数来增加条件分支预测的准确性，cpu 会提前装载后面的指令，
     * 遇到条件转移指令时会提前预测并装载某个分支的指令。编译器会产生相应的代码来优化 cpu 执行效率
     * 见 https://blog.csdn.net/wwwlyj123321/article/details/107905388
     * if (__builtin_expect(t_cachedTid == 0, 0)) 相当于 if (t_cachedTid == 0) */
    inline int tid() {
        if (__builtin_expect(t_cachedTid == 0, 0)) {
            cacheTid();
        }
        return t_cachedTid;
    }
    inline const char* tidString() { return t_tidString; } //! for logging
    inline int tidStringLength() { return t_tidStringLength; } //! for logging
    inline const char* name { return t_threadName; };

    bool isMainThread();
    void sleepUsec(int64_t usec);  //! for test
    string stackTrace(bool demangle);   //! 栈跟踪， demangle: 拆解

} // namespace CurrentThread
} // muduo

#endif //LIBZV_CURRENT_THREAD_H
