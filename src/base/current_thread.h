//
// Created by fzy on 23-3-20.
//

//! 后续可以考虑用 std::thread 替代，虽然用 std::thread无法在top中看到tid的一致

#ifndef LIBZV_CURRENT_THREAD_H
#define LIBZV_CURRENT_THREAD_H

#include "types.h"

namespace muduo {
namespace CurrentThread {
    extern __thread int t_cachedTid;   //! __thread 有什么用，需要看书
    extern __thread char t_tidString[32];
    extern __thread int t_tidStringLength;
    extern __thread const char* t_threadName;
    void cacheTid();

    inline int tid() {
        if (__builtin_expect(t_cachedTid == 0, 0)) {  //! 啥意思？
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
