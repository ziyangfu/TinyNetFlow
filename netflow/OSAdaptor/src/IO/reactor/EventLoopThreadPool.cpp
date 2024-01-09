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

EventLoopThreadPool::EventLoopThreadPool(std::shared_ptr<EventLoop> baseLoop, const std::string &name)
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
        loops_.push_back(t->startLoop());
    }
    /** 单Reactor模式， 默认 */
    if(numThreads_ == 0 && cb) {
        cb(baseLoop_);
    }
}
/*!
 * \brief 采用RR调度策略进行loop选择
 * */
std::shared_ptr<EventLoop> EventLoopThreadPool::getNextLoop() {
    baseLoop_->assertInLoopThread();
    assert(started_);
    std::shared_ptr<EventLoop> loop = baseLoop_;

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

std::shared_ptr<EventLoop> EventLoopThreadPool::getLoopForHash(size_t hashCode) {
    //baseLoop_->assertInLoopThread();
    std::shared_ptr<EventLoop> loop = baseLoop_;

    if (!loops_.empty())
    {
        loop = loops_[hashCode % loops_.size()];
    }
    return loop;
}

std::vector<std::shared_ptr<EventLoop>> EventLoopThreadPool::getAllLoops() {
    //baseLoop_->assertInLoopThread();
    assert(started_);
    if (loops_.empty())
    {
        return std::vector<std::shared_ptr<EventLoop>>(1, baseLoop_);
    }
    else
    {
        return loops_;
    }
}

