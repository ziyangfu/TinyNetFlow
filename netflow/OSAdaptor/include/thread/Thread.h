/*!
 * \brief 线程的资源管理， 基于pthread，部分参照了 std::thread的写法
 *
 * */
#ifndef OSADAPTOR_THREAD_THREAD_H
#define OSADAPTOR_THREAD_THREAD_H

#include <pthread.h>
#include <cstdint>
#include <string>
#include <optional>
#include <functional>
#include <memory>
#include <stdexcept>

#include <cstring>
#include <future>

#include "spdlog/spdlog.h"
#include "thread/Constants.h"

namespace osadaptor::thread {

/*!
 * \brief 栈大小、线程名、线程调度策略、线程优先级、线程亲和性CPU设置
 * */
struct ThreadSettings {
    std::string threadName;
    std::size_t stackSize = kDefaultThreadStackSize;

    int threadPolicy = SCHED_OTHER;  /** SCHED_OTHER(CFS),SCHED_RR,SCHED_FIFO */
    int threadPriority = 0;
    int threadAffinityCpuSet = 0;
};




/** 1. 线程的创建（静态函数创建一个线程，实例化一个Thread对象，该对象持有该线程的ID）
 *  2. 线程的配置
 *  3. 执行函数的可变参数（可变参数模板）
 *  */
class Thread final {
public:
    using threadFunc = std::function<void()>;
    struct ThreadData {
        threadFunc func;
        pthread_t threadHandler_;
    };

    Thread() = default;
    Thread(const Thread&) = delete;
    Thread& operator=(const Thread&) = delete;
    Thread(Thread&&) = default;
    Thread& operator=(Thread&&) = default;
    ~Thread();
    /*!
     * \brief 线程创建，执行给定的_Callable task，不给予线程配置信息，使用默认
     * \details 这里涉及到一种C++模板的高级用法：SFINAE。
     * _Callable不能是ThreadSettings， setting是写在函数的参数里的
     * [C++ 中复杂却很有意思的SFINAE技术](https://www.luozhiyun.com/archives/744)
     * \attention
     *      模板的声明与实现均放在.h文件中，不能分开放在.cpp中，不然会有链接错误
     *      更多查看：[原理：C++为什么一般把模板实现放入头文件](https://www.cnblogs.com/zpcdbky/p/16329886.html)
     * */
    template<typename Callable,
             std::enable_if_t<!std::is_same<std::decay_t<Callable>, ThreadSettings>::value>* = nullptr,
             typename... Args>
    static Thread create(Callable&& func, Args&&... args) {
        ThreadSettings defaultSettings;
        create(defaultSettings, std::forward<Callable>(func), std::forward<Args>(args)...);
    }
    /*!
     * \brief 线程创建，执行给定的_Callable task，给定线程配置信息 */
    template<typename Callable, typename ...Args>
    static Thread create(ThreadSettings const& settings, Callable&& func, Args&&... args) {
        pthread_attr_t attr;
        pthread_t thread_t;
        std::string trimmedName;

        // 初始化线程属性
        int ret = ::pthread_attr_init(&attr);
        if (ret != 0) {
            // 属性初始化失败处理
            return Thread(); // 返回空线程对象或抛出异常
        }
        if (!settings.threadName.empty()) {
            // 如果名称长度超过限制，则截断
            if (settings.threadName.size() > kMaxThreadNameLength) {
                trimmedName = settings.threadName.substr(0, kMaxThreadNameLength);
            }
            else {
                trimmedName = settings.threadName;
            }
            // 设置线程名称
            ret = ::pthread_setname_np(thread_t, trimmedName.c_str());
            if (ret != 0) {
                SPDLOG_ERROR("Failed to set thread name, ret: {}", ret);
            }
        }
        // 设置线程栈大小
        if (settings.stackSize > 0) {
            ret = ::pthread_attr_setstacksize(&attr, settings.stackSize);
            if (ret != 0) {
                // 栈大小设置失败处理
                ::pthread_attr_destroy(&attr);
                return Thread();
            }
        }
        // 设置调度策略（如 SCHED_OTHER, SCHED_FIFO, SCHED_RR）
//        if (!settings.threadPolicy) {
//            struct sched_param param;
//            param.sched_priority = settings.threadPriority; // 设置优先级
//
//            ret = ::pthread_attr_setschedpolicy(&attr, settings.threadPolicy);
//            if (ret != 0) {
//                ::pthread_attr_destroy(&attr);
//                return Thread();
//            }
//            // 设置调度优先级
//            ret = ::pthread_attr_setschedparam(&attr, &param);
//            if (ret != 0) {
//                ::pthread_attr_destroy(&attr);
//                return Thread();
//            }
//        }

        /** 设置 CPU 亲和性, 仅仅 Linux 支持，当前 netflow 仅支持 Linux */
//#ifdef __linux__
        if (!settings.threadAffinityCpuSet) {
            cpu_set_t cpuset;
            CPU_ZERO(&cpuset);
            CPU_SET(settings.threadAffinityCpuSet, &cpuset); // 指定运行的 CPU 核心

            ret = ::pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpuset);
            if (ret != 0) {
                ::pthread_attr_destroy(&attr);
                return Thread();
            }
        }
//#endif


//        using TaskType = std::packaged_task<decltype(func(args...))()>;
//        auto task = std::make_shared<TaskType>(
//                std::bind(std::forward<Callable>(func), std::forward<Args>(args)...)
//        );

//        int rc = pthread_create(&thread, nullptr, &Thread::entryPoint, task);
//        if (rc != 0) {
//            throw std::runtime_error("Failed to create thread");
//        }
        // 创建线程
        /** FIXME: 这么写不能支持多参数与lambda表达式 */
        //ret = ::pthread_create(&thread_t, &attr, func, &args...);
        //ret = ::pthread_create(&thread_t, &attr, &Thread::entryPoint, task);


        auto data = std::make_shared<ThreadData>();
        data->func = std::bind(std::forward<Callable>(func), std::forward<Args>(args)...);
        ret = ::pthread_create(&data->threadHandler_, &attr, &Thread::entryPoint, data.get());
        if (ret != 0) {
            ::pthread_attr_destroy(&attr);
            return Thread();
        }
        /** 销毁属性结构体 */
        ::pthread_attr_destroy(&attr);
        /** 类内静态函数是可以调用私有构造函数的，只有构造完成后才有this指针
         * 类成员函数不可调用是因为没有this指针
         * */
        Thread thread(data, settings);
        return thread;
    }


    void join();
    bool joinable() const noexcept;
    void detach();

    std::string& getThreadName();
    void setThreadName(std::string const& newThreadName);
    pthread_t getThreadId() const noexcept;
    /*!
     * \brief 用给定的线程（pid_t）替换掉当前线程
     * */
    void swap(Thread& rhs) noexcept;

private:
    Thread(std::shared_ptr<ThreadData>& data, ThreadSettings const& settings);

    static void* entryPoint(void* arg) {
//        auto task = std::static_pointer_cast<std::packaged_task<void()>>(
//                std::shared_ptr<void>((std::shared_ptr<void>*)arg));
//        (*task)();
        auto data = static_cast<ThreadData*>(arg);
        data->func(); // 执行绑定的函数
        return nullptr;
    }

    /*!
     * \brief 真正调用 pthread_create 创建线程，之前的普通函数、变参函数、lambda表达式经处理后调用该函数
     * */
    static int ptCreate(pthread_t& handler, pthread_attr_t& attr, void* (*func)(void*), void* arg);
private:
    //pthread_t threadHandler_;
    ThreadSettings settings_;
    std::shared_ptr<ThreadData> data_;
};  // class Thread

/****************************************************************************************/
inline void swap(Thread& lhs, Thread& rhs) noexcept {
    lhs.swap(rhs);
}

inline bool operator==(const Thread& lhs, const Thread& rhs) noexcept {
    return lhs.getThreadId() == rhs.getThreadId();
}

inline bool operator!=(const Thread& lhs, const Thread& rhs) noexcept {
    return !(lhs == rhs);
}

inline bool operator<(const Thread& lhs, const Thread& rhs) noexcept {
    return lhs.getThreadId() < rhs.getThreadId();
}

inline bool operator<=(const Thread& lhs, const Thread& rhs) noexcept {
    return !(rhs.getThreadId() < lhs.getThreadId());
}
/****************************************************************************************/

}  // namespace osadaptor::thread

#endif //OSADAPTOR_THREAD_THREAD_H


/**
线程错误
    1. 线程创建失败
    2. 线程所需资源不够
    3. 权限问题
    4. 配置问题
    5. 线程无法join
    6. 线程无法detach
    7. 线程名字超限（设置了名字长度）
    8. 其他未知错误

 */

/*

// 适配器类
class ThreadAdapter {
public:
    using Task = std::function<void()>;

    static void* run(void* arg) {
        auto task = reinterpret_cast<Task*>(arg);
        (*task)();
        delete task;
        return nullptr;
    }

    static pthread_t createThread(const Task& task) {
        pthread_t thread;
        auto* heapTask = new Task(task);

        int ret = pthread_create(&thread, nullptr, &ThreadAdapter::run, heapTask);
        if (ret != 0) {
            delete heapTask;
            // 处理错误（抛异常或返回空值）
        }

        return thread;
    }
};


 */