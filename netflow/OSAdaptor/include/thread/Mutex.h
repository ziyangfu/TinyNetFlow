/*!
 * \brief 封装 std::mutex，增加一些断言功能
 * */

#ifndef OSADAPTOR_THREAD_MUTEX_H
#define OSADAPTOR_THREAD_MUTEX_H

#include <mutex>
#include <thread>
#include <cassert>

class Mutex {
public:
    void lock()
    {
        mutex_.lock();
        owner_ = std::this_thread::get_id();
    }

    void unlock()
    {
        owner_ = std::thread::id();
        mutex_.unlock();
    }

    bool isLockedByThisThread() const
    {
        return owner_ == std::this_thread::get_id();
    }

private:
    std::mutex mutex_;
    std::thread::id owner_;
};

/** sample：
 *
    void someFunctionThatRequiresLock()
    {
        assert(mutex_.isLockedByThisThread());
        // your code here
}
 * */




#endif //OSADAPTOR_THREAD_MUTEX_H
