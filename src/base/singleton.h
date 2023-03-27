//
// Created by fzy on 23-3-27.
//

//!  单例模式

#ifndef LIBZV_SINGLETON_H
#define LIBZV_SINGLETON_H

#include "noncopyable.h"

#include <assert.h>
#include <pthread.h>
#include <stdlib.h>  //! atexit
//! 啥意思？？？
namespace muduo::detail {
// This doesn't detect inherited member functions!
// http://stackoverflow.com/questions/1966362/sfinae-to-check-for-inherited-member-functions
template<typename T>
struct has_no_destroy {
    template<class C> static char test(decltype(&C::no_destroy));
    template<class C> static int32_t test(...);
    const static bool value = sizeof(test<T>(0)) == 1;  //! (sizeof(test<T>(0)) == 1)的结果 true / false ==> value
};
} // namespace muduo::detail

namespace muduo {

template<typename T>
class Singleton : Noncopyable {
public:
    Singleton() = delete;
    ~Singleton() = delete;

    static T& instance() {
        pthread_once(&ponce_, &Singleton::init);
        assert(value_ != NULL);
        return *value_;
    }
private:
    static void init() {
        value_ = new T();
        if (!detail::has_no_destroy<T>::value) {
            ::atexit(destroy);  /* Register a function to be called when `quick_exit' is called.  */
        }
    }

    static void destroy() {
        using T_must_be_complete_type = char[sizeof(T) == 0 ? -1: 1];
        T_must_be_complete_type dummy; (void) dummy;   //! 这句话啥意思？？

        delete value_;
        value_ = NULL;  //! value_ == nullptr 更好？
    }

private:
    static pthread_once_t ponce_;
    static T* value_;
};

template<typename T>
pthread_once_t  Singleton<T>::ponce_ = PTHREAD_ONCE_INIT;  //! 赋初值？

template<typename T>
T* Singleton<T>::value_ = NULL;

} // namespace muduo

#endif //LIBZV_SINGLETON_H

/*! chatGPT 注释版：
namespace muduo::detail {
    // 定义了一个辅助结构体模板，用于检查类型 T 是否包含 no_destroy 静态成员变量
    template<typename T>
    struct has_no_destroy {
        // 定义了一个成员函数模板 test，该函数可以接受一个类型参数 C，并返回 char 类型的值，
        // 用于检查 C 类型是否包含 no_destroy 静态成员变量
        template<class C> static char test(decltype(&C::no_destroy));
        // 定义了一个成员函数模板 test，该函数可以接受任意类型的参数，返回 int32_t 类型的值
        template<class C> static int32_t test(...);
        // 该静态成员变量表示 T 类型是否包含 no_destroy 静态成员变量，如果包含，则其值为 true，否则其值为 false
        const static bool value = sizeof(test<T>(0)) == 1;
    };
} // namespace muduo::detail

namespace muduo {
    // 定义了一个单例模板类
    template<typename T>
    class Singleton : Noncopyable {
    public:
        Singleton() = delete;
        ~Singleton() = delete;

        // 返回单例对象的引用，如果单例对象还没有被创建，则创建之
        static T& instance() {
            pthread_once(&ponce_, &Singleton::init);
            assert(value_ != NULL);
            return *value_;
        }
    private:
        // 初始化单例对象，如果单例类型不包含 no_destroy 静态成员变量，则在程序退出时自动销毁单例对象
        static void init() {
            value_ = new T();
            if (!detail::has_no_destroy<T>::value) {
                ::atexit(destroy);
            }
        }

        // 销毁单例对象，并释放对象所占用的内存空间
        static void destroy() {
            // 使用 sizeof(T) == 0 ? -1: 1; 的技巧可以检查 T 类型是否是完整的类型
            // 如果 T 类型是不完整的类型，则编译器会在这里报错
            using T_must_be_complete_type = char[sizeof(T) == 0 ? -1: 1];
            T_must_be_complete_type dummy; (void) dummy;

            delete value_;
            value_ = NULL;
        }

    private:
        // 表示单例对象是否已经被创建，用于实现线程安全的单例模式
        static pthread_once_t ponce_;
        // 表示单例对象的指针
        static T* value_;
    };

    // 初始化 pthread_once_t 类型的静态成员变量
    template<typename T>
    pthread_once_t Singleton<T>::ponce_ = PTHREAD_ONCE_INIT;

    // 初始化单例对象指针的静态成员变量
    template<typename T>
    T* Singleton<T>::value_ = NULL;
}  // namespace muduo

*/
