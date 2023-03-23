//
// Created by fzy on 23-3-22.
//

#ifndef LIBZV_MUTEX_H
#define LIBZV_MUTEX_H

#include "current_thread.h"
#include "noncopyable.h"

#include <assert.h>
#include <pthread.h>

/*! --------------------------- 直接复制 muduo -------------------------------------------------- */
/*!
 * muduo 抄的下面链接的 Clang 官方代码
 * Clang 线程安全分析是一种C++语言扩展，可警告代码中潜在的竞争条件。分析是完全静态的（即编译时）；
 * 没有运行时开销。该分析仍在积极开发中，但已经足够成熟，可以部署在工业环境中。它由 Google 与 CERT/SEI 合作开发，
 * 并广泛用于 Google 的内部代码库 */
// Thread safety annotations {
// https://clang.llvm.org/docs/ThreadSafetyAnalysis.html

// Enable thread safety attributes only with clang.
// The attributes can be safely erased when compiling with other compilers.
#if defined(__clang__) && (!defined(SWIG))
#define THREAD_ANNOTATION_ATTRIBUTE__(x)   __attribute__((x))
#else
#define THREAD_ANNOTATION_ATTRIBUTE__(x)   // no-op
#endif

#define CAPABILITY(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(capability(x))

#define SCOPED_CAPABILITY \
  THREAD_ANNOTATION_ATTRIBUTE__(scoped_lockable)

#define GUARDED_BY(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(guarded_by(x))

#define PT_GUARDED_BY(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(pt_guarded_by(x))

#define ACQUIRED_BEFORE(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(acquired_before(__VA_ARGS__))

#define ACQUIRED_AFTER(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(acquired_after(__VA_ARGS__))

#define REQUIRES(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(requires_capability(__VA_ARGS__))

#define REQUIRES_SHARED(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(requires_shared_capability(__VA_ARGS__))

#define ACQUIRE(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(acquire_capability(__VA_ARGS__))

#define ACQUIRE_SHARED(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(acquire_shared_capability(__VA_ARGS__))

#define RELEASE(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(release_capability(__VA_ARGS__))

#define RELEASE_SHARED(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(release_shared_capability(__VA_ARGS__))

#define TRY_ACQUIRE(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(try_acquire_capability(__VA_ARGS__))

#define TRY_ACQUIRE_SHARED(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(try_acquire_shared_capability(__VA_ARGS__))

#define EXCLUDES(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(locks_excluded(__VA_ARGS__))

#define ASSERT_CAPABILITY(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(assert_capability(x))

#define ASSERT_SHARED_CAPABILITY(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(assert_shared_capability(x))

#define RETURN_CAPABILITY(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(lock_returned(x))

#define NO_THREAD_SAFETY_ANALYSIS \
  THREAD_ANNOTATION_ATTRIBUTE__(no_thread_safety_analysis)

// End of thread safety annotations }

#ifdef CHECK_PTHREAD_RETURN_VALUE

#ifdef NDEBUG
__BEGIN_DECLS
extern void __assert_perror_fail (int errnum,
                                  const char *file,
                                  unsigned int line,
                                  const char *function)
    noexcept __attribute__ ((__noreturn__));
__END_DECLS
#endif

#define MCHECK(ret) ({ __typeof__ (ret) errnum = (ret);         \
                       if (__builtin_expect(errnum != 0, 0))    \
                         __assert_perror_fail (errnum, __FILE__, __LINE__, __func__);})

#else  // CHECK_PTHREAD_RETURN_VALUE

#define MCHECK(ret) ({ __typeof__ (ret) errnum = (ret);         \
                       assert(errnum == 0); (void) errnum;})

#endif // CHECK_PTHREAD_RETURN_VALUE
/*! ---------------------------  以上直接复制 muduo -------------------------------------------------- */

namespace muduo {

//! 加锁，然后拿到线程的 TID
class CAPABILITY("mutex") MutexLock : Noncopyable {
public:
    MutexLock()
        : holder_(0) {
        MCHECK(pthread_mutex_init(&mutex_, NULL));
    };
    ~MutexLock() {
        assert(holder_ == 0);
        MCHECK(pthread_mutex_destroy(&mutex_));
    }

    /*! 必须在锁上的情况下调用 */
    bool isLockedByThisThread() const {
        return holder_ == CurrentThread::tid();
    }
    void assertLocked() const ASSERT_CAPABILITY(this) {
        assert(isLockedByThisThread());
    }
    //! 仅供 MutexLockGuard类使用
    void lock() ACQUIRE() {
        MCHECK(pthread_mutex_lock(&mutex_));
        assignHolder();
    }
    //! 仅供 MutexLockGuard类使用
    void unlock() RELEASE() {
        unassignHolder();
        MCHECK(pthread_mutex_unlock(&mutex_));
    }
    void unassignHolder() {
        holder_  = 0;
    }
    void assignHolder() {
        holder_ = CurrentThread::tid();
    }
    //! 仅供 Condition 使用, 严禁用户代码调用
    pthread_mutex_t* getPthreadMutex() {
        return &mutex_;
    }
private:
    friend class Condition;  //! 友元
    /*! -------------------------------------- class UnassignGuard -------------------------------- */
    /*! TODO 功能是什么？？ */
    class UnassignGuard : Noncopyable {
    public:
        explicit UnassignGuard(MutexLock& owner)
            :owner_(owner) {
            owner_.unassignHolder();
        }
        ~UnassignGuard() {
            owner_.assignHolder();
        }
    private:
        MutexLock& owner_;
    };
    /*! ------------------------------- end of class UnassignGuard -------------------------------- */

private:
    pthread_mutex_t mutex_;
    pid_t holder_;
};

/*!
 * 功能：锁守护
 * */
class SCOPED_CAPABILITY MutexLockGuard : Noncopyable {
public:
    explicit MutexLockGuard(MutexLock& mutex) ACQUIRE(mutex)
        : mutex_(mutex) {
        mutex_.lock();
    }
    ~MutexLockGuard() {
        mutex_.unlock();
    }
private:
    MutexLock& mutex_;
};

} // namespace muduo
//! 宏的作用：防止遗漏变量名，产生了一个临时对象然后又马上销毁了，没有锁住临界区
//! 用这种静态断言也可以  #define MutexLockGuard(x) staic_assert(false, "Missing guard object name")
#define MutexLockGuard(x) error "Missing guard object name"

#endif //LIBZV_MUTEX_H
