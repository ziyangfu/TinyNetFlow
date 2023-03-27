//
// Created by fzy on 23-3-26.
//

#include "thread_pool.h"
#include "exception.h"

#include <assert.h>
#include <stdio.h>

using namespace muduo;

ThreadPool::ThreadPool(const string &nameArg)
    : mutex_(),
      notEmpty_(mutex_),
      notFull_(mutex_),
      name_(nameArg),
      maxQueueSize_(0),
      running_(false)
{
}

ThreadPool::~ThreadPool() {
    if (running_) {
        stop();
    }
}
/*!
 * \brief 批量创建线程，并将线程放入线程池 */
void ThreadPool::start(int numThreads) {
    assert(threads_.empty());
    running_ = true;
    threads_.reserve(numThreads);
    for (int i = 0; i < numThreads; ++i) {
        char id[32];
        snprintf(id, sizeof(id), "%d", i+1);
        //!  这里也可以使用 make_unique, 这句话需要重点理解
        threads_.emplace_back(new muduo::Thread(
                std::bind(&ThreadPool::runInThread, this), name_ + id ));
        threads_[i]->start();  //! 这里底层调用了 pthread_create创建线程
    }
    // TODO 在干啥
    if (numThreads == 0 && threadInitCallback_) {
        threadInitCallback_();
    }
}

void ThreadPool::stop() {
    {
        MutexLockGuard lock(mutex_);
        running_ = false;
        notEmpty_.notifyAll();
        notFull_.notifyAll();
    }  //! 析构，unlock
    for (auto& thr : threads_) {
        thr->join();
    }
}
/*!
 * \brief 在线程池不空，以及任务队列没有满的前提下，把任务放入任务队列，并通知 */
void ThreadPool::run(Task task) {
    if (threads_.empty()) {
        task();
    }
    else {
        MutexLockGuard lock(mutex_);
        while (isFull() && running_) {
            notFull_.wait();
        }
        if (!running_) return;
        assert(!isFull());

        queue_.push_back(std::move(task));  //! 将任务放入队列尾端
        notEmpty_.notify();
    }
}
/*! \brief 任务队列长度 */
size_t ThreadPool::queueSize() const {
    MutexLockGuard lock(mutex_);
    return queue_.size();
}
/*! \brief 任务队列大小是否超出 maxQueueSize_ */
bool ThreadPool::isFull() const {
    mutex_.assertLocked();  //! 确认是否锁上，在上锁的前提下使用（前面已经上锁）
    return maxQueueSize_ > 0 && queue_.size() >= maxQueueSize_;
}

/*! \brief 把任务从任务队列中取出，分配给线程运行 */
void ThreadPool::runInThread() {
    try
    {
        if (threadInitCallback_) {
            threadInitCallback_();
        }
        while (running_) {
            Task task(take());  //! 取出一个任务
            if (task) {
                task();
            }
        }
    }
    catch (const Exception& ex)
    {
        fprintf(stderr, "exception caught in ThreadPool %s\n", name_.c_str());
        fprintf(stderr, "reason: %s\n", ex.what());
        fprintf(stderr, "stack trace: %s\n", ex.stackTrace());
        abort();
    }
    catch (const std::exception& ex)
    {
        fprintf(stderr, "exception caught in ThreadPool %s\n", name_.c_str());
        fprintf(stderr, "reason: %s\n", ex.what());
        abort();
    }
    catch (...)
    {
        fprintf(stderr, "unknown exception caught in ThreadPool %s\n", name_.c_str());
        throw; // rethrow
    }

}
/*! \brief 从任务队列中拿出一个任务 task */
Task ThreadPool::take() {
    MutexLockGuard lock(mutex_);
    //! 用 while 循环，防止虚假唤醒
    while (queue_.empty() && running_) {
        notEmpty_.wait();
    }
    Task task;
    if (!queue_.empty()) {
        task = queue_.front();   //! 从头部取出任务
        queue_.pop_front();
        if (maxQueueSize_ > 0) {
            notFull_.notify();
        }
    }
    return task;
}