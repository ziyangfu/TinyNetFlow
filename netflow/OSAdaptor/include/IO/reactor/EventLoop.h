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
 *      Reactor事件循环
 * \file
 *      EventLoop.h
 * ----------------------------------------------------------------------------------------- */

#ifndef TINYNETFLOW_OSADAPTOR_EVENTLOOP_H
#define TINYNETFLOW_OSADAPTOR_EVENTLOOP_H
#include <atomic>
#include <mutex>
#include <memory>
#include <vector>
#include <functional>
#include <thread>
#include <any>

#include "time/Timestamp.h"
#include "time/TimerId.h"
#include "time/TimerQueue.h"
#include "IO/net/Callbacks.h"

namespace osadaptor::net {

class Channel;
class EpollPoller;

class EventLoop
{
public:
    using Functor = std::function<void()>; /** 上层回调函数 */
private:
    using ChannelList = std::vector<Channel*>;
private:
    std::atomic_bool looping_;
    std::atomic_bool quit_;
    std::atomic_bool eventHandling_;
    std::atomic_bool callingPendingFunctors_;
    int wakeupFd_;
    std::unique_ptr<EpollPoller> poller_;
    std::unique_ptr<Channel> wakeupChannel_;

    ChannelList activeChannels_;
    Channel* currentActiveChannel_;

    std::mutex mutex_;
    std::vector<Functor> pendingFunctors_;

    std::thread::id tid_;

    std::unique_ptr<time::TimerQueue> timerQueue_;
    time::Timestamp pollReturnTime_;
    int64_t iteration_;

public:
    EventLoop();
    ~EventLoop();

    void loop();
    void quit();
    void runInLoop(Functor cb);
    void queueInLoop(Functor cb);
    bool isInLoopThread() {
        return tid_ == std::this_thread::get_id();
    }
    std::size_t getQueueSize() const;
    /** Timer定时器 */
    time::TimerId runAt(time::Timestamp time, TimerCallback cb);
    time::TimerId runAfter(double delay, TimerCallback cb);
    time::TimerId runEvery(double interval, TimerCallback cb);
    void cancel(time::TimerId timerId);

    time::Timestamp getEpollReturnTime() const { return pollReturnTime_; }

    void wakeup();  /** 通过 event fd 唤醒 */

    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    bool hasChannel(Channel* channel);

    void assertInLoopThread();
    static EventLoop* getEventLoopOfCurrentThread();
private:
    void abortNotInLoopThread();
    void handleReadForWakeup();
    void doPendingFunctors();
};

}  // namespace osadaptor::net

#endif //TINYNETFLOW_OSADAPTOR_EVENTLOOP_H
