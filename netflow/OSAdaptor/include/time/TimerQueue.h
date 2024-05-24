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
 *      定时器队列，采用红黑树管理，自动排序，也可以使用小顶堆来做，参见TimerQueueHeap.h
 * \file
 *      TimerQueue.h
 * ----------------------------------------------------------------------------------------- */

#ifndef TINYNETFLOW_OSADAPTOR_TIMERQUEUE_H
#define TINYNETFLOW_OSADAPTOR_TIMERQUEUE_H

#include <set>
#include <vector>
#include <atomic>
#include <memory>

#include "time/Timestamp.h"
#include "IO/net/Callbacks.h"
#include "IO/reactor/Channel.h"

namespace osadaptor {

namespace net {

class EventLoop;

}  //namespace net

namespace time {

class Timer;

class TimerId;

class TimerQueue {
private:
    /*!
     *\details Timestamp + Timer： 保证唯一性， 因为可能有同一时间到期的Timer，但是 &Timer这个地址是不可能重复的,
     * 比较大小时， Timestamp中有 < 比较函数
     * std::pair也可以用元组 tuple
     * */
    using Entry = std::pair<Timestamp, Timer *>;
    /*!
     *\details TimerList用set而不是map的原因是。这里只有key，没有 value
     * */
    using TimerList = std::set<Entry>;
    using ActiveTimer = std::pair<Timer *, int64_t>;
    using ActiveTimerSet = std::set<ActiveTimer>;

    net::EventLoop *loop_;
    net::Channel timerFdChannel_;
    const int timerFd_;

    TimerList timers_;
    ActiveTimerSet activeTimers_;
    std::atomic_bool callingExpiredTimers_;
    ActiveTimerSet cancelingTimers_;
public:
    explicit TimerQueue(net::EventLoop *loop);

    ~TimerQueue();

    TimerId addTimer(net::TimerCallback cb, Timestamp when, double interval);

    void cancel(TimerId timerId);

private:
    /*!
     * \brief 在循环中添加定时器
     * \details IO线程的事情，交给IO线程做
     * */
    void addTimerInLoop(Timer *timer);

    void cancelInLoop(TimerId timerId);

    /*!
     * \brief 定时器到期是的回调处理函数 */
    void handleRead();

    std::vector<Entry> getExpired(Timestamp now);

    void reset(const std::vector<Entry> &expired, Timestamp now);

    bool insert(Timer *timer);
};

} // namespace time
} // namespace osadaptor

#endif //TINYNETFLOW_OSADAPTOR_TIMERQUEUE_H