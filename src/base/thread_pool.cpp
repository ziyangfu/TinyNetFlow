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

void ThreadPool::run(Task f) {

}

size_t ThreadPool::queueSize() const {

}

bool ThreadPool::isFull() const {

}

void ThreadPool::runInThread() {

}

Task ThreadPool::take() {

}