//
// Created by fzy on 23-3-16.
//

//! 倒计时器

#ifndef LIBZV_COUNT_DOWN_LATCH_H
#define LIBZV_COUNT_DOWN_LATCH_H

#include "condition.h"
#include "mutex.h"

namespace muduo {

class CountDownLatch : Noncopyable {
public:
    explicit CountDownLatch(int count);
    ~CountDownLatch() = default;
    void wait();
    void countDown();
    void getCount() const;
private:
    mutable MutexLock mutex_;  //! 被mutable修饰的变量，将永远处于可变的状态，即使在一个const函数中
    /*!
     * GUARDED_BY是一个应用在数据成员上的属性，它声明了数据成员被给定的监护权保护。
     * 对于数据的读操作需要共享的访问权限，而写操作需要独占的访问权限。*/
    Condition condition_ GUARDED_BY(mutex_);
    int count_ GUARDED_BY(mutex_)     //! TODO 为什么不用原子操作？
};

} // muduo

#endif //LIBZV_COUNT_DOWN_LATCH_H
