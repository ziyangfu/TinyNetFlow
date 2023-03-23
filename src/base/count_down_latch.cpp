//
// Created by fzy on 23-3-16.
//
//! 倒计时器

#include "count_down_latch.h"

namespace muduo {
    CountDownLatch::CountDownLatch(int count)
        : mutex_(),
          condition_(mutex_),
          count_(count)
    {
    }

    void CountDownLatch::wait() {
        MutexLockGuard lock(mutex_);    //! 靠构造函数加锁，靠析构函数解锁
        while (count_ > 0) {
            condition_.wait();
        }
    }  //! 析构，在这里锁被释放

    void CountDownLatch::countDown() {
        MutexLockGuard lock(mutex_);
        --count_;
        if (count_ == 0) {
            condition_.notifyAll();
        }
    }

    void CountDownLatch::getCount() const {
        MutexLockGuard lock(mutex_);
        return count_;
    }
} // muduo