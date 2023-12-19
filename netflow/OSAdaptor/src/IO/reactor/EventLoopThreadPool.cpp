//
// Created by fzy on 23-5-16.
//

#include "EventLoopThreadPool.h"
#include "EventLoopThread.h"
#include "EventLoop.h"

#include <cassert>

using namespace netflow::net;

EventLoopThreadPool::EventLoopThreadPool(netflow::net::EventLoop *baseLoop, const std::string &name)
    : baseLoop_(baseLoop),
      name_(name),
      started_(false),
      next_(0),
      numThreads_(0)
{
}

EventLoopThreadPool::~EventLoopThreadPool() {
    // PASS
}

void EventLoopThreadPool::start(const netflow::net::EventLoopThreadPool::ThreadInitCallback &cb) {
    started_ = true;
    /** 多Reactor模式 */
    for(int i = 0; i < numThreads_; ++i) {
        //char buf[name_.size() + 32];
        //snprintf(buf, sizeof buf, "%s%d", name_.c_str(), i);
        std::string buf = name_ + std::to_string(i);
        /** FIXME 不会内存泄露吗？
         * 个人理解： 线程池一直存在，当结束时，会由系统进行内存回收，所以这里不delete也可以 */
        EventLoopThread* t = new EventLoopThread(cb, buf);
        //auto t = std::unique_ptr<EventLoopThread>(buf, cb);
        threads_.push_back(std::unique_ptr<EventLoopThread>(t));
        loops_.push_back(t->startLoop());
    }
    /** 单Reactor模式， 默认 */
    if(numThreads_ == 0 && cb) {
        cb(baseLoop_);
    }
}
/*!
 * \brief 采用RR调度策略进行loop选择 */
EventLoop *EventLoopThreadPool::getNextLoop() {
    baseLoop_->assertInLoopThread();
    assert(started_);
    EventLoop* loop = baseLoop_;

    if (!loops_.empty())
    {
        // round-robin
        loop = loops_[next_];
        ++next_;
        if (static_cast<size_t>(next_) >= loops_.size())
        {
            next_ = 0;
        }
    }
    return loop;

}

EventLoop *EventLoopThreadPool::getLoopForHash(size_t hashCode) {
    //baseLoop_->assertInLoopThread();
    EventLoop* loop = baseLoop_;

    if (!loops_.empty())
    {
        loop = loops_[hashCode % loops_.size()];
    }
    return loop;
}

std::vector<EventLoop *> EventLoopThreadPool::getAllLoops() {
    //baseLoop_->assertInLoopThread();
    assert(started_);
    if (loops_.empty())
    {
        return std::vector<EventLoop*>(1, baseLoop_);
    }
    else
    {
        return loops_;
    }
}

