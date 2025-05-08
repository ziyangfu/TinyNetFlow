

#ifndef TINYNETFLOW_THREADPOOL_H
#define TINYNETFLOW_THREADPOOL_H


/**
 * 1. 任务队列 taskQueue
 * 2. 任务队列互斥锁 tqMutex
 * 3. 任务队列条件变量 tqCond
 * 4. 原子变量
 * */


namespace osadaptor::thread {

class ThreadPool {
public:
    ThreadPool() = default;
    ~ThreadPool() = default;

    void start(size_t numThreads);
    void stop();

    void addTask(std::function<void()>&& task);
    void addTask(const std::function<void()>& task);

    void waitAllTasks();

private:
    std::vector<Thread> m_threads;
    std::vector<std::function<void()>> m_tasks;
    std::mutex m_mutex;
}
}  // namespace osadaptor::thread

#endif //TINYNETFLOW_THREADPOOL_H
