/*!
 * \brief  基于pthread的线程池，复制修改自 muduo
 * */

#ifndef OSADAPTOR_THREAD_THREAD_POOL_H
#define OSADAPTOR_THREAD_THREAD_POOL_H

#include <vector>
#include <deque>
#include <string>
#include <memory>
#include <mutex>
#include <condition_variable>

#include "thread/PThread.h"

namespace osadaptor::thread {

class ThreadPool {
public:
    using Task = std::function<void ()>;

    explicit ThreadPool(const std::string& threadPoolNameArg = std::string("ThreadPool"),
               const ThreadSettings& threadSettingsArg = ThreadSettings{});
    ~ThreadPool();

    // Must be called before start().
    void setMaxTaskQueueSize(int maxSize) { maxTaskQueueSize_ = maxSize; }
    void setThreadInitCallback(const Task& cb)
    { threadInitCallback_ = cb; }
    /*!
     * \brief 创建线程池，启动所有线程
     * */
    void start(int numThreads);
    void stop();

    const std::string& getThreadPollName() const
    { return threadPoolName_; }

    size_t queueSize();

    // Could block if maxQueueSize > 0
    // Call after stop() will return immediately.
    // There is no move-only version of std::function in C++ as of C++14.
    // So we don't need to overload a const& and an && versions
    // as we do in (Bounded)BlockingQueue.
    // https://stackoverflow.com/a/25408989
    void run(Task f);

private:
    bool isFull();
    Task take();

private:
    std::string threadPoolName_;
    ThreadSettings threadSettings_;                      /** 当前线程池所有线程共享一套配置信息*/
    std::mutex mutex_;
    std::condition_variable notEmpty_;
    std::condition_variable notFull_;
    Task threadInitCallback_;
    std::vector<std::unique_ptr<PThread>> threads_;      /** 线程池 */
    std::deque<Task> queue_;                             /** 任务队列 */
    size_t maxTaskQueueSize_;
    bool running_;
};

}  // namespace osadaptor::thread

#endif //OSADAPTOR_THREAD_THREAD_POOL_H
