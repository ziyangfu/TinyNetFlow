/*!
 * \brief 使用std::thread的线程池
 * */
#ifndef OSADAPTOR_THREAD_STD_THREAD_POOL_H
#define OSADAPTOR_THREAD_STD_THREAD_POOL_H

#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <future>

namespace osadaptor::thread {

class StdThreadPool {
public:
    // 构造函数：初始化线程池
    explicit StdThreadPool(size_t numThreads) {
        for (size_t i = 0; i < numThreads; ++i) {
            workers.emplace_back([this] { this->workerThread(); });
        }
    }

    // 析构函数：清理所有线程
    ~StdThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread& worker : workers) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }

    // 提交任务到线程池
    template<typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
        using return_type = decltype(f(args...));

        auto task = std::make_shared<std::packaged_task<return_type()>>(
                std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        std::future<return_type> result = task->get_future();

        {
            std::unique_lock<std::mutex> lock(queueMutex);

            // 防止在关闭时添加新任务
            if (stop) {
                throw std::runtime_error("Enqueue on stopped ThreadPool.");
            }

            tasks.emplace([task]() { (*task)(); });
        }

        condition.notify_one();
        return result;
    }

private:
    // 工作线程主循环
    void workerThread() {
        while (true) {
            std::function<void()> task;

            {
                std::unique_lock<std::mutex> lock(queueMutex);
                condition.wait(lock, [this] { return stop || !tasks.empty(); });

                if (stop && tasks.empty()) {
                    return;
                }

                task = std::move(tasks.front());
                tasks.pop();
            }

            task();
        }
    }

    std::vector<std::thread> workers;              // 存储工作线程
    std::queue<std::function<void()>> tasks;       // 任务队列

    std::mutex queueMutex;                         // 保护任务队列的互斥锁
    std::condition_variable condition;             // 条件变量用于等待任务
    bool stop = false;                             // 线程池是否停止
};

//// 示例：使用线程池执行任务
//int main() {


}  // namespace osadaptor::thread

#endif //OSADAPTOR_THREAD_STD_THREAD_POOL_H
