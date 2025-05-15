#include <gtest/gtest.h>
#include <iostream>
#include <chrono>
#include "thread/PThread.h"
#include "thread/ThisThread.h"
#include "thread/ThreadPool.h"
#include "thread/StdThreadPool.h"

void threadFunc() {
    EXPECT_TRUE(true); // 确保进入线程执行
    std::cout << "Running in custom thread!" << std::endl;
}

void threadFuncWithArgs(const std::string& str, int i) {
    EXPECT_EQ(str, "arg1");
    EXPECT_EQ(i, 100);
    std::cout << "Running in custom thread " << str << " i = " << i << std::endl;
}
// Lambda 测试标志变量
bool lambdaExecuted = false;

// 带参数的 Lambda 测试变量
int aCaptured = 0, bCaptured = 0;

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

//TEST(ThreadTest, threadTest) {
//    std::cout << "Running thread test!" << std::endl;
//    osadaptor::thread::ThreadSettings settings;
//    //settings.threadName = "test_thread";
//    settings.stackSize = 1024 * 1024; // 1MB
//    settings.threadPolicy = SCHED_OTHER; // CFS调度
//    settings.threadPriority = 50;     // RR/FIFO 需要非零优先级
//    settings.threadAffinityCpuSet = 1; // 绑定到第一个 CPU 核心
//    /** 普通函数 */
//    auto thread1 = osadaptor::thread::PThread::create(settings, threadFunc);
//    thread1.join();
//    /** 普通函数带参数 */
//    auto thread2 = osadaptor::thread::PThread::create(settings, threadFuncWithArgs, "arg1", 100);
//    thread2.join();
//
//    /** lambda表达式 */
//    auto thread3 = osadaptor::thread::PThread::create(settings, [](void*) {
//        std::cout << "Running lambda in thread, no var " << std::endl;
//        return nullptr;
//    }, nullptr);
//    thread3.join();
//
//    auto func_lambda = [](int a, int b) {
//    std::cout << "Running lambda in thread, a = " << a << " b= " << b << std::endl;
//    };
//    auto t4 = osadaptor::thread::PThread::create(settings, func_lambda, 1, 2);
//    t4.join();
//
//    /** 普通函数，不带 thread setting */
//    auto t5 = osadaptor::thread::PThread::create(threadFunc);
//    t5.join();
//}

TEST(ThreadTest, PThreadTest) {
    std::cout << "Running PThreadTest!" << std::endl;
    osadaptor::thread::ThreadSettings settings;
    std::string name {"PThread"};

    osadaptor::thread::PThread t1(name, settings, threadFunc);
    t1.join();

    osadaptor::thread::PThread t2(name, settings, threadFuncWithArgs, "arg1", 100);
    t2.join();

    osadaptor::thread::PThread t3(name, settings, []() {
        lambdaExecuted = true;
        std::cout << "Running lambda in thread" << std::endl;
    });
    t3.join();
    EXPECT_TRUE(lambdaExecuted);

    auto func_lambda = [](int a, int b) {
        aCaptured = a;
        bCaptured = b;
        std::cout << "Running lambda in thread, a = " << a << " b= " << b << std::endl;
    };
    osadaptor::thread::PThread t4(name, settings, func_lambda, 1, 2);
    t4.join();
    EXPECT_EQ(aCaptured, 1);
    EXPECT_EQ(bCaptured, 2);
}

TEST(ThreadTest, threadPoolTest) {
    osadaptor::thread::ThreadPool pool("mainPthreadPool");
    pool.setMaxTaskQueueSize(10);
    pool.start(4);
    pool.run([]() {
        std::cout << "Running task in thread pool" << std::endl;
    });
}

TEST(ThreadTest, stdThreadPoolTest) {
    std::cout << "Running std thread pool test!" << std::endl;
    osadaptor::thread::StdThreadPool pool(4);  // 创建包含4个线程的线程池

    // 提交多个任务
    for (int i = 0; i < 8; ++i) {
        pool.enqueue([i]() {
            std::cout << "Task " << i << " is running on thread "
                      << std::this_thread::get_id() << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        });
    }

    std::cout << "All tasks are enqueued." << std::endl;
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}