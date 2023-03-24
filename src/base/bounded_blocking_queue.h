//
// Created by fzy on 23-3-24.
//
//! 有界阻塞队列实现

#ifndef LIBZV_BOUNDED_BLOCKING_QUEUE_H
#define LIBZV_BOUNDED_BLOCKING_QUEUE_H

#include "condition.h"
#include "mutex.h"

#include <boost/circular_buffer.hpp>  //! 环形缓冲区容器
#include <assert.h>

namespace muduo {

template<typename T>
class BoundedBlockingQueue : Noncopyable {
public:
    explicit BoundedBlockingQueue(int maxSize)
        : mutex_(),
          notEmpty_(mutex_),
          notFull_(mutex_),
          queue_(maxSize)
          {}
    //! push 将任务放入队列
    void put(const T& x) {
        MutexLockGuard lock(mutex_);
        while (queue_.full()) {  //! 队列满就等待
            notFull_.wait();
        }
        assert(!queue_.full());  //! 保证不满
        queue_.push_back(x);
        notEmpty_.notify();
    }

    void put (T&& x) {  //! 右值？
        MutexLockGuard lock(mutex_);
        while (queue_.full())
        {
            notFull_.wait();
        }
        assert(!queue_.full());
        queue_.push_back(std::move(x));
        notEmpty_.notify();
    }
    //! pop 将队列中的任务取出
    T  take () {
        MutexLockGuard lock(mutex_);
        while (queue_.empty()) {
            notEmpty_.wait();
        }
        assert(!queue_.empty()); //! 保证不空
        T front(std::move(queue_.front()));   //! 取出一个任务
        queue_.pop_front();
        notFull_.notify();
        return front;
    }

    //! 判断空
    bool empty() const {
        MutexLockGuard lock(mutex_);
        return queue_.empty();
    }

    //! 判断满
    bool full() const {
        MutexLockGuard lock(mutex_);
        return queue_.full();
    }

    //! 返回当前元素的数量
    size_t size() const {
        MutexLockGuard lock(mutex_);
        return queue_.size();
    }
    //! 返回容器当前可用的内部存储空间大小，即deque此时分配了多少内存空间
    size_t capacity() const {
        MutexLockGuard lock(mutex_);
        return queue_.capacity();
    }

private:
    mutable MutexLock mutex_;
    Condition notEmpty_ GUARDED_BY(mutex_);
    Condition notFull_ GUARDED_BY(mutex_);
    boost::circular_buffer<T> queue_ GUARDED_BY(mutex_);  //! 环形缓冲区
};

} // namespace muduo
#endif //LIBZV_BOUNDED_BLOCKING_QUEUE_H
