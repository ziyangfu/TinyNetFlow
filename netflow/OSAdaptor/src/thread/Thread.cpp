
#include "thread/Thread.h"

/*!
 * \private
 * */
namespace osadaptor::thread {
Thread::Thread(std::shared_ptr<ThreadData>& data, const osadaptor::thread::ThreadSettings &settings)
    : data_(data),
      settings_(settings)
{
}
Thread::~Thread() noexcept {
    if  (joinable()) {
        join();
    }
}

void Thread::join() {
    ::pthread_join(data_->threadHandler_, nullptr);
}

bool Thread::joinable() const noexcept {
    return !(data_->threadHandler_ == pthread_t{});
}

void Thread::detach() {
    ::pthread_detach(data_->threadHandler_);
}

pthread_t Thread::getThreadId() const noexcept {
    return data_->threadHandler_;
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
    int ret = ::pthread_setname_np(data_->threadHandler_, settings_.threadName.c_str());
    if (ret != 0) {
        SPDLOG_ERROR("Failed to set thread name, ret: {}", ret);
    }
}


/** static */int Thread::ptCreate(pthread_t &handler, pthread_attr_t &attr,
                     void *(*func)(void *), void *arg) {
    int ret = ::pthread_create(&handler, &attr, func, arg);
    if (ret != 0) {
        SPDLOG_ERROR("Failed to create thread, ret: {}", ret);
    }
    return ret;
}

}  // namespace osadaptor::thread