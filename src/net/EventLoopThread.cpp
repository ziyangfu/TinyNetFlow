//
// Created by fzy on 23-5-16.
//

#include "EventLoopThread.h"
#include "EventLoop.h"

#include "../base/Logging.h"

#include <assert.h>

using namespace netflow::net;

EventLoopThread::EventLoopThread(const netflow::net::EventLoopThread::ThreadInitCallback &cb,
                                 const std::string &name)
                 : loop_(nullptr),
                   exiting_(false),
                   callback_(cb),
                   ready_(false)
{
    STREAM_TRACE << "EventLoopThread";
}

EventLoopThread::~EventLoopThread() {
    STREAM_TRACE << "dtor: ~EventLoopThread()";
    exiting_ = true;
    if (loop_) {
        STREAM_TRACE << "dtor: ~EventLoopThread(), loop_ = " << loop_;
        loop_->quit();
        thread_->join();
        //join();
    }
    if (thread_ && thread_->joinable()) {
        thread_->join();
    }
    STREAM_TRACE << "end of ~EventLoopThread()";
}
/**
 * \brief 主线程
 * \return IO线程 loop 指针 */
EventLoop *EventLoopThread::startLoop() {
    /** 启动线程 */
    STREAM_TRACE << "Start loop thread";
    assert(thread_.get() == nullptr);
    thread_.reset(new std::thread(std::bind(&EventLoopThread::threadFunc, this)));

    EventLoop* loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (!loop_){
            /** 避免虚假唤醒，只有条件ready_变为true时才可以唤醒 */
            cond_.wait(lock, [this]{ return ready_; });
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
        ready_ = true;
        loop_ = &loop;
        cond_.notify_one();
    }
    loop.loop();  /** 开始循环 */

    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = nullptr;
}

void EventLoopThread::join() {
    std::unique_lock<std::mutex> lock(mutex_);
    if (thread_ && thread_->joinable()) {
        STREAM_TRACE << " thread = " << thread_->get_id() << " joinable";
        try {
            thread_->join();
        }
        catch (const std::system_error& e) {
            STREAM_ERROR << "caught a system error : " << e.what() << " code = " << e.code();
        }
        thread_.reset();
    }

}