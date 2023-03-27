//
// Created by fzy on 23-3-27.
//

//! 线程本地存储单例模式

#ifndef LIBZV_THREAD_LOCAL_SINGLETON_H
#define LIBZV_THREAD_LOCAL_SINGLETON_H

#include "noncopyable.h"

#include <assert.h>
#include <pthread.h>

namespace muduo {

template<typename T>
class ThreadLocalSingleton : Noncopyable {
public:
    ThreadLocalSingleton() = delete;
    ~ThreadLocalSingleton() = delete;

    static T* instance() {
        if (!t_value_) {
            t_value_ = new T();
            deleter_.set(t_value_);
        }
        return *t_value_;
    }

    static T* pointer() {
        return t_value_;
    }
private:
    static void destructor(void* obj) {
        assert(obj == t_value_);
        using T_must_be_complete_type = char[sizeof(T) == 0 ? -1 : 1];
        T_must_be_complete_type dummy; (void) dummy;
        delete t_value_;
        t_value_ = 0;
    }

    class Deleter {
    public:
        Deleter() {
            pthread_key_create(&pkey_, &ThreadLocalSingleton::destructor);
        }
        ~Deleter() {
            pthread_key_delete(pkey_);
        }

        void set(T* newObj) {
            assert(pthread_getspecific(pkey_) == NULL);
            pthread_setspecific(pkey_, newObj);
        }
    public:
        pthread_key_t pkey_;
    };

private:
    static __thread T* t_value_;
    static Deleter deleter_;

};

template<typename T>
__thread T* ThreadLocalSingleton<T>::t_value_ = 0;

/*! chatGPT:
 * typename ThreadLocalSingleton<T>::Deleter 表示 ThreadLocalSingleton<T> 的嵌套类 Deleter 的类型，
 * 它的定义在 ThreadLocalSingleton 类内部，用于销毁线程局部存储的对象。
 * ThreadLocalSingleton<T>::deleter_ 是 ThreadLocalSingleton<T> 的静态成员变量，用于存储 Deleter 类型的实例对象
 * ，它的定义需要在类外部进行，因为它属于类的实例化版本，并且依赖于模板参数 T 的实际类型
 */
template<typename T>
typename ThreadLocalSingleton<T>::Deleter ThreadLocalSingleton<T>::deleter_;
} // namespace muduo

#endif //LIBZV_THREAD_LOCAL_SINGLETON_H
