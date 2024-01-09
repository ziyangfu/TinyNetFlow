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
 *      EventLoopThreadPool.h
 * ----------------------------------------------------------------------------------------- */

#ifndef TINYNETFLOW_OSADAPTOR_EVENTLOOPTHREADPOOL_H
#define TINYNETFLOW_OSADAPTOR_EVENTLOOPTHREADPOOL_H

#include <functional>
#include <memory>
#include <vector>
#include <string>
namespace netflow::osadaptor::net {

class EventLoopThread;
class EventLoop;

class EventLoopThreadPool {
public:
    using ThreadInitCallback = std::function<void(std::shared_ptr<EventLoop>)>;
private:
    std::shared_ptr<EventLoop> baseLoop_;
    std::string name_;
    bool started_;
    int numThreads_; /** IO线程池中的线程数 */
    int next_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_;   /** IO线程池 */
    std::vector<std::shared_ptr<EventLoop>> loops_;

public:
    EventLoopThreadPool(std::shared_ptr<EventLoop> baseLoop, const std::string& name);

    EventLoopThreadPool(const EventLoopThreadPool& other) =delete;
    EventLoopThreadPool& operator=(const EventLoopThreadPool& other) = delete;
    EventLoopThreadPool(const EventLoopThreadPool&& other) = delete;
    EventLoopThreadPool& operator=(const EventLoopThreadPool&& other) = delete;

    ~EventLoopThreadPool();

    void setThreadNum(int numThreads) { numThreads_ = numThreads; }
    void start(const ThreadInitCallback& cb = ThreadInitCallback());
    /** 采用RR调度 */
    std::shared_ptr<EventLoop> getNextLoop();
    std::shared_ptr<EventLoop> getLoopForHash(size_t hashCode);
    std::vector<std::shared_ptr<EventLoop>> getAllLoops();

    bool isStarted() const { return started_; }
    const std::string getName() const { return name_; }
};

} // namespace netflow::osadaptor::net



#endif //TINYNETFLOW_OSADAPTOR_EVENTLOOPTHREADPOOL_H
