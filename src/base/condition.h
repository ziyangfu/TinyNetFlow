//
// Created by fzy on 23-3-22.
//
//! pthreads 条件变量
#ifndef LIBZV_CONDITION_H
#define LIBZV_CONDITION_H

#include "mutex.h"
#include <pthread.h>

namespace muduo {

class Condition : Noncopyable {
public:
    explicit Condition(MutexLock& mutex)
        : mutex_(mutex) {
        MCHECK(pthread_cond_init(&pcond_, NULL));
    }
    ~Condition() {
        MCHECK(pthread_cond_destroy(&pcond_));
    }
    void wait() {
        MutexLock::UnassignGuard ug(mutex_);
        MCHECK(pthread_cond_wait(&pcond_, mutex_.getPthreadMutex()));
    }
    //! 超时时返回 true, 否则返回 false
    bool waitForSeconds(double seconds);
    void notify() { return MCHECK(pthread_cond_signal(&pcond_)); }
    void notifyAll() { return MCHECK(pthread_cond_broadcast(&pcond_)); }
private:
    MutexLock& mutex_;
    pthread_cond_t pcond_;
};

} // muduo

#endif //LIBZV_CONDITION_H
