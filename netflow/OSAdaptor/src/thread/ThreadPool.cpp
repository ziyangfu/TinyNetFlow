#include "thread/ThreadPool.h"
#include "thread/PThread.h"
#include "thread/ThisThread.h"
#include "thread/ThreadErrorCode.h"

#include <exception>

namespace osadaptor::thread {

#if 1
ThreadPool::ThreadPool(const std::string &threadPoolNameArg,
                       const osadaptor::thread::ThreadSettings &threadSettingsArg)
        : threadPoolName_(threadPoolNameArg),
          threadSettings_(threadSettingsArg),
          mutex_(),
          notEmpty_(),
          notFull_(),
          maxTaskQueueSize_(0),
          running_(false)
{
}

ThreadPool::~ThreadPool()
{
    if (running_)
    {
        stop();
    }
}

void ThreadPool::start(int numThreads)
{
    auto runInThread2 = [this]()
    {
        try
        {
            if (threadInitCallback_)
            {
                threadInitCallback_();
            }
            while (running_)
            {
                Task task(take());
                if (task)
                {
                    task();
                }
            }
        }
        catch (const std::exception& ex)
        {
            fprintf(stderr, "exception caught in ThreadPool %s\n", threadPoolName_.c_str());
            fprintf(stderr, "reason: %s\n", ex.what());
            abort();
        }
        catch (...)
        {
            fprintf(stderr, "unknown exception caught in ThreadPool %s\n", threadPoolName_.c_str());
            throw; // rethrow
        }
    };

    assert(threads_.empty());
    running_ = true;
    threads_.reserve(static_cast<unsigned long>(numThreads));

    for (int i = 0; i < numThreads; ++i)
    {
        std::string mThreadName = "thread_" + std::to_string(i + 1);
        threads_.emplace_back(std::make_unique<PThread>(mThreadName, threadSettings_, runInThread2));
//        threads_.emplace_back(std::make_unique<PThread>(mThreadName, threadSettings_,
//                                                        std::bind(&ThreadPool::runInThread, this)));
    }
    if (numThreads == 0 && threadInitCallback_)
    {
        threadInitCallback_();
    }
}

void ThreadPool::stop()
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        running_ = false;
        notEmpty_.notify_all();
        notFull_.notify_all();
    }
    for (auto& thr : threads_)
    {
        thr->join();
    }
}

size_t ThreadPool::queueSize()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
}

void ThreadPool::run(Task task)
{
    if (threads_.empty())
    {
        task();
    }
    else
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (isFull() && running_)
        {
            //notFull_.wait(lock);
            notFull_.wait(lock, [this]()
            {
                return !running_ || !isFull();
            });
        }
        if (!running_) return;
        assert(!isFull());

        queue_.push_back(std::move(task));
        notEmpty_.notify_one();
    }
}

ThreadPool::Task ThreadPool::take()
{
    std::unique_lock<std::mutex> lock(mutex_);
    /** 使用while循环，防止虚假唤醒 */
    while (queue_.empty() && running_)
    {
        notEmpty_.wait(lock);
    }
    Task task;
    if (!queue_.empty())
    {
        task = queue_.front();
        queue_.pop_front();
        if (maxTaskQueueSize_ > 0)
        {
            notFull_.notify_one();
        }
    }
    return task;
}

bool ThreadPool::isFull()
{
    return maxTaskQueueSize_ > 0 && queue_.size() >= maxTaskQueueSize_;
}



#endif

}   // namespace osadaptor::thread
