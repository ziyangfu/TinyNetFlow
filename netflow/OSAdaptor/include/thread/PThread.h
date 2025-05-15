/*!
 * \brief 线程的资源管理， 基于pthread，部分参照了 std::thread的写法
 *        task支持普通函数、变参函数、变参lambda表达式等
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
 * \brief 必选： 栈大小
 *        可选： 线程调度策略、线程优先级、线程亲和性CPU设置
 * \details
 *          threadPolicy only support SCHED_OTHER(CFS),SCHED_RR,SCHED_FIFO
 * */
struct ThreadSettings {
    std::size_t stackSize = kDefaultThreadStackSize;
    std::optional<int> threadPolicy;
    std::optional<int> threadPriority;
    std::optional<int> threadAffinityCpuSet;

    std::string threadPolicyToString() const {
        if (threadPolicy.has_value()) {
            switch (threadPolicy.value()) {
                case SCHED_OTHER:
                    return "SCHED_OTHER(CFS)";
                case SCHED_RR:
                    return "SCHED_RR";
                case SCHED_FIFO:
                    return "SCHED_FIFO";
                case SCHED_BATCH:
                    return "SCHED_BATCH";
                case SCHED_IDLE:
                    return "SCHED_IDLE";
                    default:
                        return "other policy";
            }
        }
        else {
            return "no policy setting";
        }
    }
};

/** 1. 线程的创建（静态函数创建一个线程，实例化一个Thread对象，该对象持有该线程的ID）
 *  2. 线程的配置
 *  3. 执行函数的可变参数（可变参数模板）
 *  */
class PThread final {
public:
    using threadFunc = std::function<void()>;
    struct ThreadData {
        threadFunc func;
        pthread_t threadHandler_;
    };

    // usage: osadaptor::thread::Thread t1(settings, threadFunc);
    template<class Callable,
            std::enable_if_t<!std::is_same<std::decay_t<Callable>, ThreadSettings>::value>* = nullptr,
            class... Args>
    PThread(std::string& threadName, ThreadSettings settings, Callable&& func, Args&&... args)
        : data_(std::make_shared<ThreadData>()),
          threadName_(threadName),
          settings_(std::move(settings))
    {
        //assert(threadSettingsValid());
        data_->func = std::bind(std::forward<Callable>(func), std::forward<Args>(args)...);
        ptCreate();
    }

    /*!
     * \brief 委托构造
     * */
    template <typename Callable, typename... Args>
    explicit PThread(std::string& threadName, Callable&& func, Args&&... args)
            : PThread(threadName, ThreadSettings{},
                      std::forward<Callable>(func), std::forward<Args>(args)...) {
    }

//    PThread(const PThread&) = delete;
//    PThread& operator=(const PThread&) = delete;
    PThread(PThread&&) = default;
    PThread& operator=(PThread&&) = default;
    ~PThread();
    /*!
     * \brief 线程创建，执行给定的Callable task，不给予线程配置信息，使用默认
     * \details 这里涉及到一种C++模板的高级用法：SFINAE。
     * Callable不能是ThreadSettings， setting是写在函数的参数里的
     * [C++ 中复杂却很有意思的SFINAE技术](https://www.luozhiyun.com/archives/744)
     * \attention
     *      模板的声明与实现均放在.h文件中，不能分开放在.cpp中，不然会有链接错误
     *      为了减少.h中的实现代码，可在模板函数中处理完模板信息后，调用普通函数
     *      更多查看：[原理：C++为什么一般把模板实现放入头文件](https://www.cnblogs.com/zpcdbky/p/16329886.html)
     * */
//    template<typename Callable,
//             std::enable_if_t<!std::is_same<std::decay_t<Callable>, ThreadSettings>::value>* = nullptr,
//             typename... Args>
//    static Thread create(Callable&& func, Args&&... args) {
//        ThreadSettings defaultSettings;
//        create(defaultSettings, std::forward<Callable>(func), std::forward<Args>(args)...);
//    }
//    /*!
//     * \brief 线程创建，执行给定的_Callable task，给定线程配置信息 */
//    template<typename Callable, typename ...Args>
//    static Thread create(ThreadSettings const& settings, Callable&& func, Args&&... args) {
//        auto data = std::make_shared<ThreadData>();
//        data->func = std::bind(std::forward<Callable>(func), std::forward<Args>(args)...);
//        return ptCreate(settings, data);
//    }

    void join();
    bool joinable() const noexcept;
    void detach();

    std::string& getThreadName();
    void setThreadName(std::string const& newThreadName);
    pthread_t getThreadId() const noexcept;
    /*!
     * \brief 通过系统调用获取线程ID
     * */
    pid_t getTid() const noexcept;
    /*!
     * \brief 用给定的线程（pid_t）替换掉当前线程
     * */
    void swap(PThread& rhs) noexcept;

private:
    /*!
     * \brief 线程设置验证，OK返回 true
     * */
    bool threadSettingsValid() const;

    static void* startThread(void* arg);
    /*!
     * \brief 真正调用 pthread_create 创建线程，之前的普通函数、变参函数、lambda表达式经处理后调用该函数
     * */
    void ptCreate();
    /*!
     * \brief 真正调用 pthread_create 创建线程，之前的普通函数、变参函数、lambda表达式经处理后调用该函数
     * */
//    static Thread ptCreate(const ThreadSettings& settings, std::shared_ptr<ThreadData> data);
private:
    std::shared_ptr<ThreadData> data_;
    std::string threadName_;
    ThreadSettings settings_;
};  // class Thread

/****************************************************************************************/
inline void swap(PThread& lhs, PThread& rhs) noexcept {
    lhs.swap(rhs);
}

inline bool operator==(const PThread& lhs, const PThread& rhs) noexcept {
    return lhs.getThreadId() == rhs.getThreadId();
}

inline bool operator!=(const PThread& lhs, const PThread& rhs) noexcept {
    return !(lhs == rhs);
}

inline bool operator<(const PThread& lhs, const PThread& rhs) noexcept {
    return lhs.getThreadId() < rhs.getThreadId();
}

inline bool operator<=(const PThread& lhs, const PThread& rhs) noexcept {
    return !(rhs.getThreadId() < lhs.getThreadId());
}
/****************************************************************************************/

}  // namespace osadaptor::thread

#endif //OSADAPTOR_THREAD_THREAD_H

/**
 * Tips: std::function<void()>为何能接受多变参的函数？
 * std::function<void()> 表示的是一个无返回值、无参数的函数对象，因此
 * 它只能绑定一个无返回值，无参数的函数或者lambda表达式
 *
 * 若需要绑定一个有参数的函数，则需要使用泛型模板，通过
 * data->func = std::bind(std::forward<Callable>(func),
 *                        std::forward<Args>(args)...);
 * 将参数绑定到函数上，最终生成一个无参的std::function<void()>函数对象
 *
*/