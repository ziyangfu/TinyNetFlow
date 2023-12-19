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
 *      时间戳
 * \file
 *       Timestamp.cpp
 * ----------------------------------------------------------------------------------------- */

#include "time/Timestamp.h"
#include <cstdio>
#include <sys/time.h>
#include <cinttypes>

using namespace netflow::osadaptor::time;

static_assert(sizeof(Timestamp) == sizeof(int64_t),
              "Timestamp should be same size as int64_t");

std::string Timestamp::toString() const
{
    char buf[32] = {0};
    int64_t seconds = microSecondsSinceEpoch_ / kMicroSecondsPerSecond;
    int64_t microseconds = microSecondsSinceEpoch_ % kMicroSecondsPerSecond;
    /** 对于int64_t类型来说，处于跨平台的考虑，建议使用 PRId64 */
    snprintf(buf, sizeof(buf), "%" PRId64 ".%06" PRId64 "", seconds, microseconds);
    return buf;
}

std::string Timestamp::toFormattedString(bool showMicroseconds) const
{
    char buf[64] = {0};
    time_t seconds = static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond);
    struct tm tm_time;
    gmtime_r(&seconds, &tm_time);

    if (showMicroseconds)
    {
        int microseconds = static_cast<int>(microSecondsSinceEpoch_ % kMicroSecondsPerSecond);
        snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d.%06d",
                 tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                 tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
                 microseconds);
    }
    else
    {
        snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d",
                 tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                 tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
    }
    return buf;
}

Timestamp Timestamp::now()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int64_t seconds = tv.tv_sec;
    return Timestamp(seconds * kMicroSecondsPerSecond + tv.tv_usec);
}



#if 0
Timestamp Timestamp::now() {
    std::timespec ts;
    std::timespec_get(&ts, TIME_UTC);
    return Timestamp(ts);
}

std::string Timestamp::toFormattedString(bool isShowNanosecond){
    char buff[100] = {0};
    /*
    struct tm tm_time = std::gmtime(time_.tv_sec);

    if (isShowNanosecond) {
        snprintf(buff, sizeof buff, "%4d%02d%02d %02d:%02d:%02d.%09ld",
                 tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                 tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec, time_.tv_nsec);
    }
    else {
        snprintf(buff, sizeof buff, "%4d%02d%02d %02d:%02d:%02d",
                 tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                 tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
        //strftime(buff, sizeof buff, "%D %T", std::gmtime(&time_.tv_sec))
    }
     */
    return buff;
}
/*!
 * \brief 获取以纳秒表示的时间 */
int64_t Timestamp::getNanoseconds() {
    int64_t nanoFromSecond = static_cast<int64_t>(time_.tv_sec * kNanoSecondsPerSecond);
    int64_t nanoSecond = static_cast<int64_t>(time_.tv_nsec);
    return (nanoFromSecond + nanoSecond);
}
#endif