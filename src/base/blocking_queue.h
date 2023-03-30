//
// Created by fzy on 23-3-24.
//
//! 无界阻塞队列实现 （生产者消费者队列）

#ifndef LIBZV_BLOCKING_QUEUE_H
#define LIBZV_BLOCKING_QUEUE_H

#include "condition.h"
#include "mutex.h"

#include <deque>  //! 双端队列
#include <assert.h>

namespace muduo {
template<typename T>
class BlockingQueue : Noncopyable {
public:
    using queue_type = std::deque<T>;
    BlockingQueue()
        : mutex_(),
          notEmpty_(mutex_),
          queue_() {}
    //! 将事件放入阻塞队列？
    void put (const T& x) {
        MutexLockGuard lock(mutex_);
        queue_.push_back(x);
        notEmpty_.notify();
    }

    T take() {
        MutexLockGuard lock(mutex_);
        //! 必须用 while 循环，而不能用 if， 防止虚假唤醒
        while (queue_.empty()) {
            notEmpty_.wait();
        }
    }

    queue_type drain() {
        queue_type queue;  //! std::deque<T> queue;
        {
            MutexLockGuard lock(mutex_);
            queue = std::move(queue_);
            assert(queue_.empty());
        }  //! 锁在这里释放
        return queue;
    }
    //! 获取双端队列大小
    size_t size() const {
        MutexLockGuard lock(mutex_);
        return queue_.size();
    }
private:
    mutable MutexLock mutex_;
    Condition notEmpty_ GUARDED_BY(mutex_);
    queue_type queue_ GUARDED_BY(mutex_);
};
} // namespace muduo


#endif //LIBZV_BLOCKING_QUEUE_H
