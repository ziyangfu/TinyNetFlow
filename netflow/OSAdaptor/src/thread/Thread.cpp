
#include "thread/Thread.h"

/*!
 * \private
 * */
namespace osadaptor::thread {
Thread::Thread(pthread_t threadId, const osadaptor::thread::ThreadSettings &settings)
    : threadHandler_(threadId),
      settings_(settings)
{
}

//template<typename _Callable,
//        std::enable_if_t<!std::is_same<std::decay_t<_Callable>, ThreadSettings>::value> *,
//        typename ...Args>
///** static */ Thread Thread::create(_Callable &&func, Args &&... args) {
//    ThreadSettings defaultSettings;
//    create(defaultSettings, std::forward<_Callable>(func), std::forward<Args>(args)...);
//}
//
///*!
// * \brief 静态函数创建一个线程，实例化一个Thread对象，该对象持有该线程的ID
// * */
//template<typename _Callable, typename... Args>
///** static */Thread Thread::create(ThreadSettings const& settings, _Callable&& func, Args&&... args) {
//    pthread_attr_t attr;
//    pthread_t thread_t;
//    std::string trimmedName;
//
//    // 初始化线程属性
//    int ret = ::pthread_attr_init(&attr);
//    if (ret != 0) {
//        // 属性初始化失败处理
//        return Thread(); // 返回空线程对象或抛出异常
//    }
//    if (!settings.threadName.empty()) {
//        // 如果名称长度超过限制，则截断
//        if (settings.threadName.size() > kMaxThreadNameLength) {
//            trimmedName = settings.threadName.substr(0, kMaxThreadNameLength);
//        }
//        else {
//            trimmedName = settings.threadName;
//        }
//        // 设置线程名称
//        ret = ::pthread_setname_np(thread_t, trimmedName.c_str());
//        if (ret != 0) {
//            SPDLOG_ERROR("Failed to set thread name: %s", strerror(ret));
//        }
//    }
//
//    // 设置线程栈大小
//    if (settings.stackSize > 0) {
//        ret = ::pthread_attr_setstacksize(&attr, settings.stackSize);
//        if (ret != 0) {
//            // 栈大小设置失败处理
//            ::pthread_attr_destroy(&attr);
//            return Thread();
//        }
//    }
//
//    // 设置调度策略（如 SCHED_OTHER, SCHED_FIFO, SCHED_RR）
//    if (!settings.threadPolicy) {
//        struct sched_param param;
//        param.sched_priority = settings.threadPriority; // 设置优先级
//
//        ret = ::pthread_attr_setschedpolicy(&attr, settings.threadPolicy);
//        if (ret != 0) {
//            ::pthread_attr_destroy(&attr);
//            return Thread();
//        }
//        // 设置调度优先级
//        ret = ::pthread_attr_setschedparam(&attr, &param);
//        if (ret != 0) {
//            ::pthread_attr_destroy(&attr);
//            return Thread();
//        }
//    }
//
//    /** 设置 CPU 亲和性, 仅仅 Linux 支持，当前 netflow 仅支持 Linux */
////#ifdef __linux__
//    if (!settings.threadAffinityCpuSet) {
//        cpu_set_t cpuset;
//        CPU_ZERO(&cpuset);
//        CPU_SET(settings.threadAffinityCpuSet, &cpuset); // 指定运行的 CPU 核心
//
//        ret = ::pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpuset);
//        if (ret != 0) {
//            ::pthread_attr_destroy(&attr);
//            return Thread();
//        }
//    }
////#endif
//
//    // 创建线程
//    ret = ::pthread_create(&thread_t, &attr, func, &args...);
//    if (ret != 0) {
//        ::pthread_attr_destroy(&attr);
//        return Thread();
//    }
//    /** 销毁属性结构体 */
//    ::pthread_attr_destroy(&attr);
//    /** 类内静态函数是可以调用私有构造函数的，只有构造完成后才有this指针
//     * 类成员函数不可调用是因为没有this指针
//     * */
//    Thread thread(thread_t, settings);
//    return thread;
//}

void Thread::join() {
    ::pthread_join(threadHandler_, nullptr);
}

bool Thread::joinable() const noexcept {
    return !(threadHandler_ == pthread_t{});
}

void Thread::detach() {
    ::pthread_detach(threadHandler_);
}

pthread_t Thread::getThreadId() const noexcept {
    return threadHandler_;
}

std::string& Thread::getThreadName() {
    return settings_.threadName;
}

void Thread::setThreadName(std::string const& newThreadName) {
    if (newThreadName.size() > kMaxThreadNameLength) {
        settings_.threadName = newThreadName.substr(0, kMaxThreadNameLength);
    }
    else {
        settings_.threadName = newThreadName;
    }
    int ret = ::pthread_setname_np(threadHandler_, settings_.threadName.c_str());
    if (ret != 0) {
        SPDLOG_ERROR("Failed to set thread name, ret: {}", ret);
    }
}


}  // namespace osadaptor::thread