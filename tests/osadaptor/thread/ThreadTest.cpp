#include <gtest/gtest.h>
#include <iostream>
#include <chrono>
#include "thread/Thread.h"
#include "thread/ThisThread.h"
#include "thread/ThreadPool.h"

void threadFunc(void*) {
    std::cout << "Running in custom thread!" << std::endl;
}

void threadFuncWithArgs(const std::string& str, int i) {
    std::cout << "Running in custom thread " << str << " i = " << i << std::endl;
}

TEST(ThreadTest, thisThreadSleepForTest) {
    std::cout << "Current thread name: " << osadaptor::thread::thisThread::getName() << std::endl;
    auto start = std::chrono::steady_clock::now();
    osadaptor::thread::thisThread::sleepFor(std::chrono::seconds(2));
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    // 验证休眠时间是否在合理范围内（允许±50ms误差）
    EXPECT_GE(duration, 1950); // 至少等待1950ms
    EXPECT_LE(duration, 2050); // 最多不超过2050ms
}

TEST(ThreadTest, thisThreadSleepUtilTest) {
    auto now = std::chrono::steady_clock::now();
    auto targetTime = now + std::chrono::seconds(2);

    osadaptor::thread::thisThread::sleepUtil(targetTime);
    auto end = std::chrono::steady_clock::now();

    // 验证当前时间是否已经到达或超过目标时间
    EXPECT_GE(end, targetTime - std::chrono::milliseconds(50)); // 允许提前50ms唤醒
    EXPECT_LE(end, targetTime + std::chrono::milliseconds(50)); // 允许延迟50ms
}

TEST(ThreadTest, threadTest) {
    std::cout << "Running thread test!" << std::endl;
    osadaptor::thread::ThreadSettings settings;
    settings.stackSize = 1024 * 1024; // 1MB
    settings.threadPolicy = SCHED_OTHER; // CFS调度
    settings.threadPriority = 50;     // RR/FIFO 需要非零优先级
    settings.threadAffinityCpuSet = 1; // 绑定到第一个 CPU 核心
    /** 普通函数 */
    auto thread1 = osadaptor::thread::Thread::create(settings, threadFunc);
    thread1.join();
    /** 普通函数带参数 */
    auto thread2 = osadaptor::thread::Thread::create(settings, threadFunc, "arg1", 100);
    thread2.join();

    /** lambda表达式 */
    auto thread3 = osadaptor::thread::Thread::create(settings, [](void*) {
        std::cout << "Running in custom thread!" << std::endl;
        return nullptr;
    }, nullptr);
    thread3.join();

    /** 普通函数，不带 thread setting */
    auto t4 = osadaptor::thread::Thread::create(threadFunc);
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}