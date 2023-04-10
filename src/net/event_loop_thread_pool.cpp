//
// Created by fzy on 23-3-13.
//

#include "event_loop_thread_pool.h"
#include "event_loop.h"
#include "event_loop_thread.h"

#include <stdio.h>

using namespace muduo;
using namespace muduo::net;

EventLoopThreadPool::EventLoopThreadPool(muduo::net::EventLoop *baseLoop, const std::string &nameArg)
    : baseLoop_(baseLoop),
      name_(nameArg),
      started_(false),
      numThreads_(0),
      next_(0)
{
}

EventLoopThreadPool::~EventLoopThreadPool() {

}

void EventLoopThreadPool::start(const muduo::net::EventLoopThreadPool::ThreadInitCallback &cb) {
    assert(!started_);
    baseLoop_->assertInLoopThread();
    started_ = true;

    for (int i = 0; i < numThreads_; ++i) {
        char buf[name_.size() + 32];
        snprintf(buf, sizeof(buf), "%s%d", name_.c_str(), i);
        EventLoopThread* t = new EventLoopThread(cb,buf);
        threads_.push_back(std::unique_ptr<EventLoopThread>(t));   /*! 裸指针转换为智能指针？ */
        loops_.push_back(t->startLoop());
    }
    if (numThreads_ == 0 && cb) {
        cb(baseLoop_);
    }

}

EventLoop *EventLoopThreadPool::getNextLoop() {
    baseLoop_->assertInLoopThread();
    assert(started_);
    EventLoop* loop = baseLoop_;

    if (!loops_.empty()) {
        /*! 时间片轮转 */
        loop = loops_[next_];
        ++next_;
        if (implicit_cast<size_t>(next_) >= loops_.size()) {
            next_ = 0;
        }
    }
    return loop;
}

EventLoop *EventLoopThreadPool::getLoopForHash(size_t hashCode) {
    baseLoop_->assertInLoopThread();
    EventLoop* loop = baseLoop_;

    if (!loops_.empty()) {
        loop = loops_[hashCode % loops_.size()];
    }
    return loop;
}

std::vector<EventLoop *> EventLoopThreadPool::getAllLoops() {
    baseLoop_->assertInLoopThread();
    assert(started_);

    if (loops_.empty()) {
        /*! 创建一个std::vector<EventLoop*>类型的对象，其中包含一个元素，这个元素的值为baseLoop_ */
        return std::vector<EventLoop*>(1, baseLoop_);  /*! 可以使用初始化列表替换 */
    }
    else {
        return loops_;
    }
}