//
// Created by fzy on 23-3-20.
//

#include "current_thread.h"

#include <cxxabi.h>  //! 每一个都有什么用？？
#include <execinfo.h>
#include <stdlib.h>

namespace muduo {
namespace CurrentThread {
    __thread int t_cachedTid = 0;
    __thread char t_tidString[32];
    __thread int t_tidStringLength = 6;
    __thread const char* t_threadName = "unknown";

    static_assert(std::is_same<int, pid_t>::value, "pid_t should be int");
    /*! TODO: 栈跟踪的代码还需要好好理解 */
    string stackTrace(bool demangle) {
        string stack;
        const int max_frames = 200;
        void* frame[max_frames];   //! void* 无类型指针, void 指针可以指向任意类型的数据
        int nptrs = ::backtrace(frame, max_frames);  //! execinfo.h
        char** strings = ::backtrace_symbols(frame, nptrs);
        if (strings) {
            size_t len = 256;
            char* demangled = demangle ? static_cast<char*>(::malloc(len)) : nullptr;
            for (int i = 1; i < nptrs; ++i) {
                if (demangle) {
                    char* left_par = nullptr;  //! 左括号
                    char* plus = nullptr;
                    for (char* p = strings[i]; *p; ++p) {
                        if (*p == '(') {
                            left_par = p;
                        }
                        else if (*p == '+') {
                            plus = p;
                        }
                    }
                    if (left_par && plus) {
                        *plus = '\0';
                        int status = 0;
                        char* ret = abi::__cxa_demangle(left_par + 1,
                                                        demangled, &len, &status);  //! cxxabi.h
                        *plus = '+';
                        if (status == 0) {
                            demangled = ret;
                            stack.append(strings[i], left_par + 1);
                            stack.append(demangled);
                            stack.append(plus);
                            stack.push_back('\n');
                            continue;
                        }
                    }

                }
                stack.append(strings[i]);
                stack.push_back('\n');
            }
            free(demangled);
            free(strings)
        }
        return stack;
    }

} // namespace CurrentThread
} // muduo