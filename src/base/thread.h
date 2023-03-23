//
// Created by fzy on 23-3-16.
//
//! pthreads 线程， Linux 底层线程库

#ifndef LIBZV_THREAD_H
#define LIBZV_THREAD_H

#include "atomic.h"
#include "count_down_latch.h"
#include "types.h"

#include <functional>
#include <memory>
#include <pthread.h>

namespace muduo {

class Thread :Noncopyable {
public:
    using ThreadFunc = std::function<void ()>;
    explicit Thread(ThreadFunc func, const string& name = string());
    ~Thread();

    void start();
    void join(); //! 返回 pthread_join();

    bool started() const { return started_; }
    pid_t tid() const { return tid_; }
    const string& name() const { return name_; }

    static int numCreated() { return numCreated_.get(); }
private:
    void setDefaultName();
private:
    bool            started_;
    bool            joined_;
    pthread_t       pthreadId_;
    pid_t           tid_;
    ThreadFunc      func_;
    string          name_;
    CountDownLatch  latch_;
    static AtomicInt32 numCreated_;
};

} // muduo

#endif //LIBZV_THREAD_H
