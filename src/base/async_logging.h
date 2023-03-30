//
// Created by fzy on 23-3-30.
//
//! 高性能异步日志库

#ifndef LIBZV_ASYNC_LOGGING_H
#define LIBZV_ASYNC_LOGGING_H

#include "blocking_queue.h"
#include "bounded_blocking_queue.h"
#include "count_down_latch.h"
#include "mutex.h"
#include "thread.h"
#include "log_stream.h"

#include <atomic>
#include <vector>

namespace muduo {
class AsyncLogging : Noncopyable {
public:
    AsyncLogging(const string& basename, off_t rollSize, int flushInterval = 3); //! 默认3秒写入一次
    ~AsyncLogging() {
        if (running_) {
            stop();
        }
    }
    void append(const char* logline, int len);

    void start() {
        running_ = true;
        thread_.start();
        latch_.wait();
    }
    void stop() NO_THREAD_SAFETY_ANALYSIS {
        running_ = false;
        cond_.notify();
        thread_.join();
    }
private:
    void threadFunc();
private:
    using Buffer = muduo::detail::FixedBuffer<muduo::detail::kLargeBuffer>;  //! template SIZE = kLargeBuffer
    using BufferVector = std::vector<std::unique_ptr<Buffer>>;
    using BufferPtr = BufferVector::value_type;    //! std::unique_ptr<FixedBuffer<SIZE>>

    const int flushInterval_;
    std::atomic<bool> running_;
    const string basename_;
    const off_t rollSize_;
    muduo::Thread thread_;
    muduo::CountDownLatch latch_;
    muduo::MutexLock mutex_;
    muduo::Condition cond_ GUARDED_BY(mutex_);
    BufferPtr currentBuffer_ GUARDED_BY(mutex_);
    BufferPtr nextBuffer_ GUARDED_BY(mutex_);
    BufferVector buffers_ GUARDED_BY(mutex_);

};
} // namespace muduo

#endif //LIBZV_ASYNC_LOGGING_H
