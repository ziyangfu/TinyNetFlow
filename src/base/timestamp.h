//
// Created by fzy on 23-3-16.
//

//! 时间戳

#ifndef LIBZV_TIMESTAMP_H
#define LIBZV_TIMESTAMP_H

//! 是否可以用 std::chrono 替换？？
#include "copyable.h"
#include "types.h"

#include <boost/operators.hpp>  //! 后期考虑替换

namespace muduo {
/*! ------------------------------------ class Timestamp ------------------------------------------------------- */
class Timestamp : public muduo::Copyable,
                  public boost::equality_comparable<Timestamp>,
                  public boost::less_than_comparable<Timestamp>
{
public:
    //! 构造一个无效的 Timestamp
    Timestamp()
        : microSecondsSinceEpoch_(0){};
    //! 在特定时间构造一个 Timestamp
    /*! @param microSecondSinceEpochArg */
    explicit Timestamp(int64_t microSecondSinceEpochArg)
        : microSecondsSinceEpoch_(microSecondSinceEpochArg){}

    void swap(Timestamp& that) {
        std::swap(microSecondsSinceEpoch_, that.microSecondsSinceEpoch_);
    }
    string toString() const;
    string toFormattedString(bool showMicroseconds = true) const;

    bool valid() const { return microSecondsSinceEpoch_ > 0; }

    //! 内部使用
    int64_t microSecondsSinceEpoch() const { return microSecondsSinceEpoch_; }
    time_t secondsSinceEpoch() const { return static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond); }
    //! 获取当前时间
    static Timestamp now();
    static Timestamp invalid() { return Timestamp(); }
    //! unix时间： 从UTC1970年1月1日0时0分0秒起至现在的总秒数，不考虑闰秒
    static Timestamp fromUnixTime(time_t t) {
        return fromUnixTime(t, 0);
    }
    static Timestamp fromUnixTime(time_t t, int microseconds) {
        return Timestamp(static_cast<int64_t>(t) * kMicroSecondsPerSecond + microseconds);
    }

public:
    static const int kMicroSecondsPerSecond = 1000 * 1000;

private:
    int64_t microSecondsSinceEpoch_;
};
/*! ------------------------------ end of class Timestamp ------------------------------------------------------- */
//! 重载 2 个运算符
inline bool operator<(Timestamp lhs, Timestamp rhs) {
    return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}
inline bool operator==(Timestamp lhs, Timestamp rhs) {
    return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
}
//! 以秒为单位返回
inline double timeDifference(Timestamp high, Timestamp low) {
    int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
    return static_cast<double>(diff) / Timestamp::kMicroSecondsPerSecond;
}
inline Timestamp addTime(Timestamp timestamp, double seconds) {
    int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
    return Timestamp(timestamp.microSecondsSinceEpoch() + delta);
}
} // muduo

#endif //LIBZV_TIMESTAMP_H
