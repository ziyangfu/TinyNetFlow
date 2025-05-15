#include "thread/PThread.h"
#include "spdlog/spdlog.h"
/*!
 * \private
 * */
namespace osadaptor::thread {

PThread::~PThread() noexcept {
    if  (joinable()) {
        join();
    }
}

void PThread::join() {
    if (joinable()) {
        ::pthread_join(data_->threadHandler_, nullptr);
    }

}

/*!
 * \details
 *      threadHandler_不等于一个空句柄，则表示线程仍然活跃，并且未被join或detach，即它是joinable的。
 * */
bool PThread::joinable() const noexcept {
    return !(data_->threadHandler_ == pthread_t{});
}

void PThread::detach() {
    if (joinable()) {
        ::pthread_detach(data_->threadHandler_);
    }

}

pthread_t PThread::getThreadId() const noexcept {
    return data_->threadHandler_;
}

pid_t PThread::getTid() const noexcept {
    return static_cast<pid_t>(::syscall(SYS_gettid));
}

std::string& PThread::getThreadName() {
    return threadName_;
}

void PThread::setThreadName(std::string const& newThreadName) {
    if (newThreadName.size() > kMaxThreadNameLength) {
        threadName_ = newThreadName.substr(0, kMaxThreadNameLength);
    }
    else {
        threadName_ = newThreadName;
    }
    int ret = ::pthread_setname_np(data_->threadHandler_, threadName_.c_str());
    if (ret != 0) {
        SPDLOG_ERROR("Failed to set thread name, ret: {}", ret);
    }
}

void PThread::swap(osadaptor::thread::PThread &rhs) noexcept {
    std::swap(data_, rhs.data_);
    std::swap(settings_, rhs.settings_);
}

void PThread::ptCreate() {
    pthread_attr_t attr;
    std::string trimmedName;

    // 初始化线程属性
    int ret = ::pthread_attr_init(&attr);
    if (ret != 0) {
        // 属性初始化失败处理
        //return Thread(); // 返回空线程对象或抛出异常
    }
    if (!threadName_.empty()) {
        // 如果名称长度超过限制，则截断
        if (threadName_.size() > kMaxThreadNameLength) {
            trimmedName = threadName_.substr(0, kMaxThreadNameLength);
        }
        else {
            trimmedName = threadName_;
        }
    }
    // 设置线程栈大小
    if (settings_.stackSize > 0) {
        ret = ::pthread_attr_setstacksize(&attr, settings_.stackSize);
        if (ret != 0) {
            // 栈大小设置失败处理
            ::pthread_attr_destroy(&attr);
            //return Thread();
        }
    }
    if (settings_.threadPolicy.has_value()) {
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
    }
    /** 调度优先级的设置与调度策略有关，RT类调度策略如RR,FIFO与 CFS类调度策略的优先级是不一样的 */
    if (settings_.threadPriority.has_value()) {
        /** FIXME: (@fzy )*/
    }

    /** 设置 CPU 亲和性, 仅仅 Linux 支持，当前 netflow 仅支持 Linux */
    if (settings_.threadAffinityCpuSet.has_value()) {
#ifdef __linux__
        if (!settings_.threadAffinityCpuSet) {
            cpu_set_t cpuset;
            CPU_ZERO(&cpuset);
            CPU_SET(settings_.threadAffinityCpuSet.value(), &cpuset); // 指定运行的 CPU 核心

            ret = ::pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpuset);
            if (ret != 0) {
                ::pthread_attr_destroy(&attr);
                //return Thread();
            }
        }
#endif
    }
    ret = ::pthread_create(&data_->threadHandler_, &attr, &PThread::startThread, data_.get());
    if (ret != 0) {
        ::pthread_attr_destroy(&attr);
    }
    // 设置线程名称
    ret = ::pthread_setname_np(data_->threadHandler_, trimmedName.c_str());
    if (ret != 0) {
        SPDLOG_ERROR("Failed to set thread name, ret: {}, thread name is {}， strerr: {}",
                     ret, trimmedName, strerror(ret));
    }
    /** 销毁属性结构体 */
    ::pthread_attr_destroy(&attr);
}


bool PThread::threadSettingsValid() const {
    bool valid {false};
    if (threadName_.size() > kMaxThreadNameLength) {
        return valid;
    }
    if (settings_.stackSize > kMaxThreadStackSize) {
        return valid;
    }
    if (settings_.threadPolicy.has_value()) {
        if (settings_.threadPolicy != SCHED_OTHER &&
            settings_.threadPolicy != SCHED_FIFO  &&
            settings_.threadPolicy != SCHED_RR) {
            SPDLOG_ERROR("Invalid thread policy {}", settings_.threadPolicyToString());
            return valid;
        }
    }
    if (settings_.threadPriority.has_value()) {
        /** FIXME: (@fzy )*/
    }
    if (settings_.threadAffinityCpuSet.has_value()) {
        /** FIXME: (@fzy )*/
    }
    return true;
}

/*!
 * \details
 *      为什么startThread必须是static的？
 *      因为pthread_create的第三个参数类型为：void *(*)(void *)
 *      而类成员函数隐含一个this指针，即类型为：void *(Thread::*)(void *, Thread *this)
 *      类型不匹配，因此需要使用static修饰，否则编译器会报错。
 * */
/** static */void *PThread::startThread(void *arg) {
    auto data = static_cast<ThreadData*>(arg);   // ThreadData的原始指针
    data->func(); // 执行绑定的函数， bind了完美转发的callable与args
    return nullptr;
}

#if 0
/** static */Thread Thread::ptCreate(const ThreadSettings& settings, std::shared_ptr<ThreadData> data) {
    pthread_attr_t attr;
    pthread_t thread_t;
    std::string trimmedName;

    // 初始化线程属性
    int ret = ::pthread_attr_init(&attr);
    if (ret != 0) {
        // 属性初始化失败处理
        //return Thread(); // 返回空线程对象或抛出异常
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
            //return Thread();
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
            //return Thread();
        }
    }
//#endif

    ret = ::pthread_create(&data->threadHandler_, &attr, &Thread::entryPoint, data.get());
    if (ret != 0) {
        ::pthread_attr_destroy(&attr);
        //return Thread();
    }
    /** 销毁属性结构体 */
    ::pthread_attr_destroy(&attr);

    /** 类内静态函数是可以调用私有构造函数的，只有构造完成后才有this指针
     * 类成员函数不可调用是因为没有this指针
     * */
    Thread thread(data, settings);
    return thread;
}
#endif

}  // namespace osadaptor::thread