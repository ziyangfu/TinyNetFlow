/** ----------------------------------------------------------------------------------------
 * \copyright
 * Copyright (c) 2023 by the TinyNetFlow project authors. All Rights Reserved.
 *
 * This file is open source software, licensed to you under the ter；ms
 * of the Apache License, Version 2.0 (the "License").  See the NOTICE file
 * distributed with this work for additional information regarding copyright
 * ownership.  You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 * -----------------------------------------------------------------------------------------
 * \brief
 *      多Reactor与单Reactor模式，多Reactor模式将创建线程池
 * \file
 *      EventLoopThreadPool.cpp
 * ----------------------------------------------------------------------------------------- */

#include "IO/reactor/EventLoopThreadPool.h"
#include "IO/reactor/EventLoopThread.h"
#include "IO/reactor/EventLoop.h"

#include <cassert>

using namespace netflow::osadaptor::net;

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop, std::string_view name)
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

void EventLoopThreadPool::start(const EventLoopThreadPool::ThreadInitCallback &cb) {
    started_ = true;
    /** 多Reactor模式 */
    for(int i = 0; i < numThreads_; ++i) {
        //char buf[name_.size() + 32];
        //snprintf(buf, sizeof buf, "%s%d", name_.c_str(), i);
        std::string threadName = name_ + std::to_string(i);
        /** unique_ptr没有=，若先make_unique，再push_back会出错，
         * 除了以下方法，还可以使用emplace_back来原地构造
         * */
        auto t = new EventLoopThread(cb, threadName);
        threads_.push_back(std::unique_ptr<EventLoopThread>(t));
        loops_.push_back(t->startLoop());  /** 创建线程，并绑定EventLoop */
    }
    /** 单Reactor模式， 默认， 实际只执行一个回调函数，整个进程只有一个主线程 */
    if(numThreads_ == 0 && cb) {
        cb(baseLoop_);
    }
}
/*!
 * \brief 采用RR调度策略进行loop选择
 * */
EventLoop* EventLoopThreadPool::getNextLoop() {
    baseLoop_->assertInLoopThread();
    assert(started_);
    EventLoop* loop = baseLoop_;   /** 对于单Reactor来说，就是baseLoop */

    if (!loops_.empty())        /** 对于多Reactor来说，RR调度 */
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

EventLoop* EventLoopThreadPool::getLoopForHash(size_t hashCode) {
    //baseLoop_->assertInLoopThread();
    EventLoop* loop = baseLoop_;

    if (!loops_.empty())
    {
        loop = loops_[hashCode % loops_.size()];
    }
    return loop;
}

std::vector<EventLoop*> EventLoopThreadPool::getAllLoops() {
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

