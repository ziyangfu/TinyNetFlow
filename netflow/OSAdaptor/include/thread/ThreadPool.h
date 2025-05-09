

#ifndef TINYNETFLOW_THREADPOOL_H
#define TINYNETFLOW_THREADPOOL_H


/**
 * 1. 任务队列 taskQueue
 * 2. 任务队列互斥锁 tqMutex
 * 3. 任务队列条件变量 tqCond
 * 4. 原子变量
 * */
#include <vector>
#include <string>
#include <mutex>

namespace osadaptor::thread {

class ThreadPool {
//public:
//    typedef std::function<void ()> Task;
//
//    explicit ThreadPool(const string& nameArg = string("ThreadPool"));
//    ~ThreadPool();
//
//    // Must be called before start().
//    void setMaxQueueSize(int maxSize) { maxQueueSize_ = maxSize; }
//    void setThreadInitCallback(const Task& cb)
//    { threadInitCallback_ = cb; }
//
//    void start(int numThreads);
//    void stop();
//
//    const string& name() const
//    { return name_; }
//
//    size_t queueSize() const;
//
//    // Could block if maxQueueSize > 0
//    // Call after stop() will return immediately.
//    // There is no move-only version of std::function in C++ as of C++14.
//    // So we don't need to overload a const& and an && versions
//    // as we do in (Bounded)BlockingQueue.
//    // https://stackoverflow.com/a/25408989
//    void run(Task f);
//
//private:
//    bool isFull() const REQUIRES(mutex_);
//    void runInThread();
//    Task take();
//
//    mutable MutexLock mutex_;
//    Condition notEmpty_ GUARDED_BY(mutex_);
//    Condition notFull_ GUARDED_BY(mutex_);
//    string name_;
//    Task threadInitCallback_;
//    std::vector<std::unique_ptr<muduo::Thread>> threads_;
//    std::deque<Task> queue_ GUARDED_BY(mutex_);
//    size_t maxQueueSize_;
//    bool running_;

};
}  // namespace osadaptor::thread

#endif //TINYNETFLOW_THREADPOOL_H
