//
// Created by fzy on 23-6-14.
//
#include "../EventLoopThread.h"
#include "../EventLoop.h"

#include "../../base/Logging.h"

#include <thread>
#include <chrono>

#include <unistd.h>

using namespace netflow::base;
using namespace netflow::net;

void print(EventLoop* p = nullptr)
{
    STREAM_INFO << " pid = " << getpid() << " tid = " << std::this_thread::get_id()
                << " EventLoop point addr = " << p;
}

void quit(EventLoop* p)
{
    print(p);
    p->quit();
}

int main()
{
    Logger::get().set_level(spdlog::level::info);  /** trace 查看网络库消息 */
    print();
    /** 测试1： EventLoopThread类是否可以正确构造与析构 */
    {
        EventLoopThread thr1;  // never start
    }
    /** 测试2： 当主线程先结束，IO线程后结束，即EventLoopThread类先析构，EventLoop类后析构，是否正常 */
    {
        // dtor calls quit()
        EventLoopThread thr2;
        EventLoop* loop = thr2.startLoop();
        loop->runInLoop(std::bind(print, loop));
    }
    /** 测试3： 当IO线程先结束，主线程后结束，即EventLoop类先析构， EventLoopThread类后析构，是否正常 */
    {
        // quit() before dtor
        EventLoopThread thr3;
        EventLoop* loop = thr3.startLoop();
        /** IO线程提前停止 */
        loop->runInLoop(std::bind(quit, loop));
        std::chrono::duration duration = std::chrono::seconds(5);  /** 非正规做法 */
        std::this_thread::sleep_for(duration);
    }

}

