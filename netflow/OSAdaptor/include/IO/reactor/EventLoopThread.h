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
 *      one loop per thread, 为EventLoop所有者
 * \file
 *      EventLoopThread.h
 * ----------------------------------------------------------------------------------------- */

#ifndef TINYNETFLOW_OSADAPTOR_EVENTLOOPTHREAD_H
#define TINYNETFLOW_OSADAPTOR_EVENTLOOPTHREAD_H

#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <memory>
#include <string>

namespace netflow::osadaptor::net {

class EventLoop;

class EventLoopThread {
public:
    using ThreadInitCallback = std::function<void(std::shared_ptr<EventLoop>&)>;
private:
    std::shared_ptr<EventLoop> loop_;  /** EventLoopThread为EventLoop 所有者 */
    bool exiting_;
    bool ready_;
    std::mutex mutex_;
    std::shared_ptr<std::thread> thread_;
    std::condition_variable cond_;
    ThreadInitCallback callback_;

public:
    EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(), const std::string& name = std::string());
    ~EventLoopThread();
    std::shared_ptr<EventLoop> startLoop();
private:
    void threadFunc();
    void join();
};

}  // namespace netflow::osadaptor::net



#endif //TINYNETFLOW_OSADAPTOR_EVENTLOOPTHREAD_H
