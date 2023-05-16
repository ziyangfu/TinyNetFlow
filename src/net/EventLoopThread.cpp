//
// Created by fzy on 23-5-16.
//

#include "EventLoopThread.h"
#include "EventLoop.h"

using namespace netflow::net;

EventLoopThread::EventLoopThread(const std::string &name,
                                 const netflow::net::EventLoopThread::ThreadInitCallback &cb)
                 : loop_(nullptr),
                   exiting_(false),
                   mutex_(),
                   cond_(mutex_),
                   callback_(cb)
{
}

EventLoopThread::~EventLoopThread() {
    exiting_ = true;
    if (!loop_){
        loop_->quit();
        thread_->join();
    }
}
/**
 * \brief 主线程
 * \return IO线程 loop 指针 */
EventLoop *EventLoopThread::startLoop() {
    /** 启动线程 */
    thread_(std::make_shared<std::thread>(std::bind(&EventLoopThread::threadFunc, this)));

    EventLoop* loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (!loop_){
            cond_.wait();
        }
        loop = loop_;
    }
    return loop;

}
/**
 * \brief 子线程，启动loop，并把loop的地址给了主线程 */
void EventLoopThread::threadFunc() {
    EventLoop loop;
    if(callback_){
        callback_(&loop);
    }
    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }
    loop.loop();  /** 开始循环 */

    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = nullptr;
}