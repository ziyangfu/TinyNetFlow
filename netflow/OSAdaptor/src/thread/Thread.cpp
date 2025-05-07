
#include "thread/Thread.h"

namespace osadaptor::thread {


template<typename _Callable, typename ...Args>
Thread Thread::create() {
    void* args;
    void* (*func)(void*);
    std::size_t stackSizeBytes = 0;


    pthread_attr_t attr;
    pthread_t thread_t;
    int ret = ::pthread_attr_init(&attr);
    if (ret == 0) {
        ::pthread_attr_setstacksize(&attr, stackSizeBytes);
    }
    ::pthread_create(&thread_t, &attr, func, args);
}


void Thread::join() {
    pthread_t thread_t;
    ::pthread_join(thread_t, nullptr);
}


void Thread::getThreadName() {
    std::string threadName{};
    int const ret = pthread_getname_np(XXX, threadName.data(), threadName.size());
    if (ret != 0) {
        threadName = "unknown";
    }
    return threadName;
}


void Thread::setThreadName() {
    ::pthread_setname_np();
}


}  // namespace osadaptor::thread