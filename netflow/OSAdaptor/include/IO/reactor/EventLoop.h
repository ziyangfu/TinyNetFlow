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

#include "netflow/OSAdaptor/include/time/Timestamp.h"
#include "netflow/OSAdaptor/include/time/TimerId.h"
#include "netflow/OSAdaptor/include/IO/net/Callbacks.h"

using namespace netflow::base;

namespace netflow::net {

class Channel;
class EpollPoller;
class TimerQueue;

class EventLoop {
public:
    using Functor = std::function<void()>; /** 上层回调函数 */
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
    TimerId runAt(Timestamp time, TimerCallback cb);
    TimerId runAfter(double delay, TimerCallback cb);
    TimerId runEvery(double interval, TimerCallback cb);
    void cancel(TimerId timerId);

    Timestamp getEpollReturnTime() const { return pollReturnTime_; }

    void wakeup();  /** 通过 event fd 唤醒 */


    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    bool hasChannel(Channel* channel);

    void assertInLoopThread()
    {
        if (!isInLoopThread())
        {
            abortNotInLoopThread();
        }
    }
    static EventLoop* getEventLoopOfCurrentThread();

private:
    void abortNotInLoopThread();

    void handleReadForWakeup();
    void doPendingFunctors();
private:
    std::atomic_bool looping_;
    std::atomic_bool quit_;
    std::atomic_bool eventHandling_;
    std::atomic_bool callingPendingFunctors_;

    int wakeupFd_;

    std::unique_ptr<EpollPoller> poller_;
    std::unique_ptr<Channel> wakeupChannel_;
    using ChannelList = std::vector<Channel*>;
    ChannelList activeChannels_;
    Channel* currentActiveChannel_;

    std::mutex mutex_;
    std::vector<Functor> pendingFunctors_;

    std::thread::id tid_;

    std::unique_ptr<TimerQueue> timerQueue_;
    Timestamp pollReturnTime_;
    int64_t iteration_;

};

} // namespace netflow::net
#endif //TINYNETFLOW_OSADAPTOR_EVENTLOOP_H
