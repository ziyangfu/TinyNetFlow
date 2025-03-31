/*!
 * \brief 使用 glibc 的 context上下文切换
 * x86/AMD64 ARM/ARM64
 * libco只切换了寄存器和栈顶指针，对比ucontext少了浮点数上下文与sigmask（信号屏蔽掩码）
 * 据网友实测，libco的上下文切换性能是ucontext的3.6倍
 * */

/**
 *
typedef struct ucontext_t
  {
    unsigned long int __ctx(uc_flags);  // 存储上下文标志位
    struct ucontext_t *uc_link;         // 指向另一个 ucontext_t 结构体，用于链接多个上下文
    stack_t uc_stack;                   // 描述当前上下文使用的栈信息
    mcontext_t uc_mcontext;             // 存储机器寄存器状态
    sigset_t uc_sigmask;                // 表示信号屏蔽字
    struct _libc_fpstate __fpregs_mem;  // 保存浮点寄存器状态
    __extension__ unsigned long long int __ssp[4];      // 扩展字段
  } ucontext_t;

 * */


#ifndef OSADAPTOR_COROUTINE_SYS_CONTEXT_H
#define OSADAPTOR_COROUTINE_SYS_CONTEXT_H


#include <ucontext.h>

namespace osadaptor::coroutine {
namespace sysContext {
int getContext(ucontext_t *__ucp);
int setContext(const ucontext_t *__ucp);
int contextSwap(ucontext_t *__restrict __oucp,
                const ucontext_t *__restrict __ucp);
void makeContext(ucontext_t *__ucp, void (*__func) (void),
                 int __argc, ...);

}  // namespace sysContext
} // namespace osadaptor::coroutine

#endif //OSADAPTOR_COROUTINE_SYS_CONTEXT_H
