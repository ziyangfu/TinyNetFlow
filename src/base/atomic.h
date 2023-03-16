//
// Created by fzy on 23-3-16.
//
//! 原子操作

#ifndef LIBZV_ATOMIC_H
#define LIBZV_ATOMIC_H

#include "noncopyable.h"

#include <stdint.h>

namespace muduo {
namespace detail {
template<typename T>
class AtomicIntegerT : Noncopyable {
public:
    AtomicIntegerT()
    : value_{0}{}
    ~AtomicIntegerT();

    T get(){
        return __sync_val_compare_and_swap(&value_, 0, 0); //! GCC 内置原子操作 如果与0[第一个]相等 则将0[第二个]写入  不等则不写
    }
    T getAndAdd(T x){
        return __sync_fetch_and_add(&value_, x); //! value_ += x 返回原值
    }
    T addAndGet(T x){
        return getAndAdd(x) + x;   //! 返回新值
    }
    T incrementAndGet(){
        return addAndGet(1); //! 返回新值
    }
    T decrementAndGet(){
        return addAndGet(-1); //! 返回新值
    }

    void add(T x){
        return getAndAdd(x);
    }
    void increment() {
        return incrementAndGet();
    }
    void decrement(){
        return decrementAndGet();
    }
    T getAndSet(T newValue){
        return __sync_lock_test_and_set(&value_, newValue);  //! 将*ptr设为value并返回*ptr操作之前的值
    }
private:
    volatile T value_;
    // volatile的作用： 作为指令关键字，确保本条指令不会因编译器的优化而省略，且要求每次直接读值。
    // 简单地说就是防止编译器对代码进行优化

    // 当要求使用volatile 声明的变量的值的时候，系统总是重新从它所在的内存读取数据，而不是使用保存在寄存器中的备份。
    // 即使它前面的指令刚刚从该处读取过数据。而且读取的数据立刻被保存
};
}  // detail
using AtomicInt32 = detail::AtomicIntegerT<int32_t>;
using AtomicInt64 = detail::AtomicIntegerT<int64_t>;
} // muduo

#endif //LIBZV_ATOMIC_H
