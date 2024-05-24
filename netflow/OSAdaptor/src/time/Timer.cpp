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
 *      Timer.cpp
 * ----------------------------------------------------------------------------------------- */

#include "time/Timer.h"

using namespace osadaptor::net;
using namespace osadaptor::time;

static std::atomic_int64_t s_numCreated_ = 0;

/*!
 * \param cb : 定时器回调函数
 * \param when： 到什么时候为止时的时间戳， 触发闹钟
 * \param interval： 周期性闹钟，间隔时间为多少
 * */
 Timer::Timer(TimerCallback cb, Timestamp when, double interval)
         : callback_(std::move(cb)),
           expiration_(when),
           interval_(interval),
           repeat_(interval > 0.0),
           sequence_(++::s_numCreated_)
 {
 }

/*!
 * \brief 重启定时器，即将到期时间的时间戳设置为 now + interval 后的时间戳
 * */
void Timer::restart(Timestamp now) {
    if (repeat_) {
       expiration_ = addTime(now, interval_);
    }
    else {
        expiration_ = Timestamp::invalid();
    }
}