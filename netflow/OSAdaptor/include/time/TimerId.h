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
 *      Timer 的 ID
 * \file
 *      TimerId.h
 * ----------------------------------------------------------------------------------------- */

#ifndef TINYNETFLOW_OSADAPTOR_TIMERID_H
#define TINYNETFLOW_OSADAPTOR_TIMERID_H

#include <cstdint>
#include <memory>

namespace osadaptor::time {

class Timer;

class TimerId {
private:
    Timer* timer_;
    int64_t sequence_;

public:
    TimerId(Timer* timer, int64_t seq)
        : timer_(timer),
          sequence_(seq)
    {}
    ~TimerId() = default;
    /** public 友元： 可以访问 public和private
     *  private 友元： 可以访问 public， 不能访问 private */
    friend class TimerQueue;

};
}  // namespace osadaptor::time

#endif //TINYNETFLOW_OSADAPTOR_TIMERID_H
