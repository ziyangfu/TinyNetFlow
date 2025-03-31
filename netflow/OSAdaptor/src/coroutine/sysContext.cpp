/*!
 * \brief 封装 glibc 的 ucontext， 用于协程上下文切换
 * */

#include "coroutine/sysContext.h"

namespace osadaptor::coroutine {

int sysContext::getContext(ucontext_t *ucp) {

}

int sysContext::setContext(const ucontext_t *ucp) {

}

int sysContext::contextSwap(ucontext_t *__restrict oucp, const ucontext_t *__restrict ucp) {
}


void sysContext::makeContext(ucontext_t *ucp, void (*func)(), int argc, ...) {

}

} // namespace osadaptor::coroutine