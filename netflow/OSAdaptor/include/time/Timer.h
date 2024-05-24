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
 *      定时器，可以用于定时唤醒EventLoop
 * \file
 *      Timer.h
 * ----------------------------------------------------------------------------------------- */

#ifndef TINYNETFLOW_OSADAPTOR_TIMER_H
#define TINYNETFLOW_OSADAPTOR_TIMER_H

#include <atomic>
#include "IO/net/Callbacks.h"
#include "time/Timestamp.h"

namespace osadaptor::time {

class Timer {
private:
    /*!
     * callback_    : 定时器到期时的回调处理函数
     * expiration_  : 到期时间的时间戳
     * interval_    : 周期性定时器的时间间隔
     * repeat_      : 是否为周期性定时器
     * sequence_    : 创建了多少个定时器，定时器序列是多少
     * s_numCreated_: 静态变量
     * */
    const net::TimerCallback callback_;
    time::Timestamp expiration_;
    const double interval_;
    const bool repeat_;
    const int64_t sequence_;
    static std::atomic_int64_t s_numCreated_;
public:
    /*!
     * \param cb : 定时器回调函数
     * \param when： 到什么时候为止时的时间戳， 触发闹钟
     * \param interval： 周期性闹钟，间隔时间为多少
     * */
    Timer(net::TimerCallback cb, time::Timestamp when, double interval);
    ~Timer() = default;

    void run() const { callback_(); }
    /*!
     * \brief 获取到期时的时间戳
     * */
    time::Timestamp getExpiration() const { return expiration_; }
    bool repeat() const { return repeat_; }
    int64_t sequence() const { return sequence_; }

    void restart(time::Timestamp now);

};
} // namespace osadaptor::time



#endif //TINYNETFLOW_OSADAPTOR_TIMER_H
