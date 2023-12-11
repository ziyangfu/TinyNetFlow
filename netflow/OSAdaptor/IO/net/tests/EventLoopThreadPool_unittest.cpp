//
// Created by fzy on 23-6-14.
//


#include "netflow/OSLayer/IO/reactor/EventLoopThreadPool.h"
#include "netflow/OSLayer/IO/reactor/EventLoop.h"
#include "netflow/Log/Logging.h"

#include <thread>

#include <stdio.h>
#include <unistd.h>

using namespace netflow::base;
using namespace netflow::net;

void print(EventLoop* p = NULL)
{
    printf("main(): pid = %d, tid = %d, loop = %p\n",
           getpid(), std::this_thread::get_id(), p);
}

void init(EventLoop* p)
{
    printf("init(): pid = %d, tid = %d, loop = %p\n",
           getpid(), std::this_thread::get_id(), p);
}

int main()
{
    print();

    EventLoop loop;
    loop.runAfter(11, std::bind(&EventLoop::quit, &loop));
    /**  测试1： 单 Rector 模式 ，此时的baseloop就是传递的loop */
    {
        printf("Single thread %p:\n", &loop);
        EventLoopThreadPool model(&loop, "single");
        model.setThreadNum(0);
        model.start(init);
        assert(model.getNextLoop() == &loop);
        assert(model.getNextLoop() == &loop);
        assert(model.getNextLoop() == &loop);
    }
    /** 测试2： 多 Rector 模式，但线程池中只有一个loop */
    {
        printf("Another thread:\n");
        EventLoopThreadPool model(&loop, "another"); /** 这里将loop传递给baseloop，但多reactor不用 */
        model.setThreadNum(1);
        model.start(init);
        EventLoop* nextLoop = model.getNextLoop();
        nextLoop->runAfter(2, std::bind(print, nextLoop));
        assert(nextLoop != &loop);  /** 线程池创建的线程loop是新的，不是传递的loop（baseloop）*/
        assert(nextLoop == model.getNextLoop());
        assert(nextLoop == model.getNextLoop());
        ::sleep(3);
    }
    /** 测试3： 多 Rector 模式，线程池中有3个线程loop */
    {
        printf("Three threads:\n");
        EventLoopThreadPool model(&loop, "three");
        model.setThreadNum(3);
        model.start(init);
        EventLoop* nextLoop = model.getNextLoop();
        nextLoop->runInLoop(std::bind(print, nextLoop));
        assert(nextLoop != &loop);
        assert(nextLoop != model.getNextLoop());
        assert(nextLoop != model.getNextLoop());
        assert(nextLoop == model.getNextLoop());
    }

    loop.loop();
}

/** 参考输出
    main(): pid = 35223, tid = 865445760, loop = (nil)
    Single thread 0x7ffd6fec8280:
    init(): pid = 35223, tid = 865445760, loop = 0x7ffd6fec8280
    Another thread:
    init(): pid = 35223, tid = 865441536, loop = 0x7f1133958c90
    main(): pid = 35223, tid = 865441536, loop = 0x7f1133958c90
    Three threads:
    init(): pid = 35223, tid = 865441536, loop = 0x7f1133958c90
    init(): pid = 35223, tid = 857048832, loop = 0x7f1133157c90
    init(): pid = 35223, tid = 848656128, loop = 0x7f1132956c90
    main(): pid = 35223, tid = 865441536, loop = 0x7f1133958c90
*/

