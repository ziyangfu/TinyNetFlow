//
// Created by fzy on 23-3-28.
//
//! 时区与夏令时

#ifndef LIBZV_TIME_ZONE_H
#define LIBZV_TIME_ZONE_H

#include "copyable.h"
#include "types.h"

#include <memory>
#include <time.h>

namespace muduo {

struct DateTime {
    DateTime() {}
    explicit DateTime(const struct tm&);
    DateTime(int _year, int _month, int _day, int _hour, int _minute, int _second)
            : year(_year), month(_month), day(_day), hour(_hour), minute(_minute), second(_second)
    {
    }
    // "2011-12-31 12:34:56"
    string toIsoString() const;

    int year = 0;     // [1900, 2500]
    int month = 0;    // [1, 12]
    int day = 0;      // [1, 31]
    int hour = 0;     // [0, 23]
    int minute = 0;   // [0, 59]
    int second = 0;   // [0, 59]
};

class TimeZone :public Copyable {
public:
    TimeZone() = default;    // an invalid timezone
    TimeZone(int eastOfUtc, const char* tzname);  // a fixed timezone
    static TimeZone UTC();
    static TimeZone China();  // Fixed at GMT+8, no DST
    static TimeZone loadZoneFile(const char* zonefile);

    bool valid() const {
        // 'explicit operator bool() const' in C++11
        return static_cast<bool>(data_);
    }

    struct DateTime toLocalTime(int64_t secondsSinceEpoch, int* utcOffset = nullptr) const;
    int64_t fromLocalTime(const struct DateTime&, bool postTransition = false) const;
    //! gmtime(3)
    static struct DateTime toUtcTime(int64_t secondsSinceEpoch);
    // timegm(3)
    static int64_t fromUtcTime(const struct DateTime&);

public:
    struct Data;
private:
    explicit TimeZone(std::unique_ptr<Data> data);
private:
    std::shared_ptr<Data> data_;
    friend class TimeZoneTestPeer;
};


} // muduo

#endif //LIBZV_TIME_ZONE_H
