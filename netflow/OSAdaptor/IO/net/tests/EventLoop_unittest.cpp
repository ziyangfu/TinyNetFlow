//
// Created by fzy on 23-6-13.
//
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <iostream>
#include <thread>


#include "netflow/OSLayer/IO/reactor/EventLoop.h"
#include "netflow/Log/Logging.h"

using namespace netflow::net;
using namespace std;

EventLoop* g_loop;

void callback() {
    cout << "callback(): pid = " << getpid() << ", tid = " << this_thread::get_id() << endl;
    /** 将出错，coredump，一个线程只能有一个EventLoop */
    EventLoop anotherLoop;
}

void threadFunc() {
    cout << "threadFunc(): pid = " << getpid() << ", tid = " << this_thread::get_id() << endl;
    assert(EventLoop::getEventLoopOfCurrentThread() == nullptr);
    EventLoop loop;
    assert(EventLoop::getEventLoopOfCurrentThread() == &loop);
    /** 定时器事件测试 */
    loop.runAfter(5.0, callback);
    loop.loop();
}

int main() {
    // Logger::get().set_level(spdlog::level::info);
    cout << "main(): pid = " << getpid() << ", tid = " << this_thread::get_id() << endl;
    assert(EventLoop::getEventLoopOfCurrentThread() == nullptr);
    EventLoop loop;
    assert(EventLoop::getEventLoopOfCurrentThread() == &loop);
    cout << "start thread " << endl;
    thread thread(threadFunc);
    loop.loop();
}

