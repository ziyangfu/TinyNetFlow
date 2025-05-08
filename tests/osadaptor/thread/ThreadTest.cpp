#include <gtest/gtest.h>
#include <iostream>
#include "thread/Thread.h"

void* threadFunc(void*) {
    std::cout << "Running in custom thread!" << std::endl;
    return nullptr;
}

void* threadFuncWithArgs(std::string& str, int i) {
    std::cout << "Running in custom thread! " << str << " i = " << i << std::endl;
    return nullptr;
}

void threadTest() {
    std::cout << "Running thread test!" << std::endl;
    osadaptor::thread::ThreadSettings settings;
    settings.stackSize = 1024 * 1024; // 1MB
    settings.threadPolicy = SCHED_OTHER; // CFS调度
    settings.threadPriority = 50;     // RR/FIFO 需要非零优先级
    settings.threadAffinityCpuSet = 1; // 绑定到第一个 CPU 核心
    /** 普通函数 */
    auto thread1 = osadaptor::thread::Thread::create(settings, threadFunc, nullptr);
    auto thread2 = osadaptor::thread::Thread::create(settings, threadFunc, "arg1", 100);
//    auto thread = osadaptor::thread::Thread::create(settings, [](void*) {
//        std::cout << "Running in custom thread!" << std::endl;
//        return nullptr;
//    }, nullptr);


}

void threadPoolTest() {

}

int main() {
    threadTest();
    threadPoolTest();
}