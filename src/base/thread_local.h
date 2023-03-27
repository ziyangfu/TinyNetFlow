//
// Created by fzy on 23-3-27.
//
//! 线程本地存储

/*!
 * 线程私有变量（Thread Local Storage）之于线程相当于静态变量之于进程，与进程变量相比是每个线程都有一份， 也就是所谓的“私有”。
 * 也可以把线程私有变量理解为key-value对，其中key是线程ID。它的主要作用是在多线程编程中避免锁竞争的开销
 *
 * chatGPT的回答：
 * 这是一个 C++ 模板类实现的 ThreadLocal 存储机制。ThreadLocal 类允许每个线程都有自己的给定变量的实例。这在多线程程序中很有用，
 * 多个线程可能访问同一个变量，但每个线程都应该有自己的副本以避免竞态条件和同步问题。
 * 该实现使用 POSIX pthread 库来管理线程局部存储。当创建 ThreadLocal 类的实例时，使用 pthread_key_create 函数创建一个唯一的键。
 * 该键用于为每个线程标识线程局部变量。
 * ThreadLocal 类有一个析构函数，在线程终止时调用。该函数负责释放线程局部变量使用的内存。
 * value() 函数用于检索当前线程的线程局部变量。如果该变量尚未为此线程创建，则使用类 T 的默认构造函数创建该变量的新实例。
 * 然后使用 pthread_setspecific 函数将新实例存储在线程局部存储中，并将指向实例的指针返回给调用者。
 * 请注意，ThreadLocal 类派生自 Noncopyable 类，这确保 ThreadLocal 类的实例不能被复制或赋值。
 * 这是因为用于管理线程局部存储的 pthread_key_t 类型不能被复制，因此在未经修改的情况下复制或赋值 ThreadLocal 类将导致错误。
 * */

#ifndef LIBZV_THREAD_LOCAL_H
#define LIBZV_THREAD_LOCAL_H

#include "mutex.h"
#include "noncopyable.h"

#include <pthread.h>

namespace muduo {

template<typename T>
class ThreadLocal : Noncopyable {
public:
    ThreadLocal() {
        MCHECK(pthread_key_create(&pkey_, &ThreadLocal::destructor));
    }
    ~ThreadLocal() {
        MCHECK(pthread_key_delete(pkey_));
    }

    /*! 获取 value值 */
    T* value() {
        T* perThreadValue = static_cast<T*>(pthread_getspecific(pkey_));
        //! 如果该变量尚未为此线程创建，则使用类 T 的默认构造函数创建该变量的新实例
        if (!perThreadValue) {
            T* newObj = new T();
            MCHECK(pthread_setspecific(pkey_, newObj));
            perThreadValue = newObj;
        }
        return *perThreadValue;
    }
private:
    static void destructor(void *x) {
        T* obj = static_cast<T*>(x);
        using T_must_be_complete_type = char[sizeof(T) == 0 ? -1 : 1];
        T_must_be_complete_type dummy; (void) dummy;
        delete obj;
    }
private:
    pthread_key_t pkey_;
};
} // namespace muduo

#endif //LIBZV_THREAD_LOCAL_H
