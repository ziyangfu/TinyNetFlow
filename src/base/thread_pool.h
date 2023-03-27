//
// Created by fzy on 23-3-26.
//
//! 线程池的实现，基于 POSIX pthreads

#ifndef LIBZV_THREAD_POOL_H
#define LIBZV_THREAD_POOL_H

#include "condition.h"
#include "mutex.h"
#include "thread.h"
#include "types.h"

#include <deque>
#include <vector>

namespace muduo {

class ThreadPool : Noncopyable {
public:
    using Task = std::function<void ()>;
    explicit ThreadPool(const string& nameArg = string("ThreadPool"));
    ~ThreadPool();

    void setMaxQueueSize(int maxSize) { maxQueueSize_ = maxSize; }
    void setTheadInitCallback(const Task& cb){ threadInitCallback_ = cb; }

    void start(int numThreads);
    void stop();
    void run(Task task);

    const string& name() const { return name_; }
    size_t queueSize() const;

private:
    bool isFull() const REQUIRES(mutex_);
    void runInThread();
    Task take();  //! pop， 从头部取出任务
private:
    mutable MutexLock mutex_;
    Condition notEmpty_ GUARDED_BY(mutex_);
    Condition notFull_ GUARDED_BY(mutex_);
    string name_;
    Task threadInitCallback_;
    std::vector<std::unique_ptr<muduo::Thread>> threads_;
    std::deque<Task> queue_ GUARDED_BY(mutex_);
    size_t maxQueueSize_;
    bool running_;
};

} // muduo

#endif //LIBZV_THREAD_POOL_H
