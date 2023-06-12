//
// Created by fzy on 23-5-26.
//

#ifndef TINYNETFLOW_TIMESTAMP_H
#define TINYNETFLOW_TIMESTAMP_H

#include <cstdint>
#include <string>

namespace netflow::base {

class Timestamp
{
public:
    ///
    /// Constucts an invalid Timestamp.
    ///
    Timestamp()
            : microSecondsSinceEpoch_(0)
    {
    }

    ///
    /// Constucts a Timestamp at specific time
    ///
    /// @param microSecondsSinceEpoch
    explicit Timestamp(int64_t microSecondsSinceEpochArg)
            : microSecondsSinceEpoch_(microSecondsSinceEpochArg)
    {
    }

    void swap(Timestamp& that)
    {
        std::swap(microSecondsSinceEpoch_, that.microSecondsSinceEpoch_);
    }

    // default copy/assignment/dtor are Okay

    std::string toString() const;
    std::string toFormattedString(bool showMicroseconds = true) const;

    bool valid() const { return microSecondsSinceEpoch_ > 0; }

    // for internal usage.
    int64_t microSecondsSinceEpoch() const { return microSecondsSinceEpoch_; }
    time_t secondsSinceEpoch() const
    { return static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond); }

    ///
    /// Get time of now.
    ///
    static Timestamp now();
    static Timestamp invalid()
    {
        return Timestamp();
    }

    static Timestamp fromUnixTime(time_t t)
    {
        return fromUnixTime(t, 0);
    }

    static Timestamp fromUnixTime(time_t t, int microseconds)
    {
        return Timestamp(static_cast<int64_t>(t) * kMicroSecondsPerSecond + microseconds);
    }

    static const int kMicroSecondsPerSecond = 1000 * 1000;

private:
    int64_t microSecondsSinceEpoch_;
};

inline bool operator<(Timestamp lhs, Timestamp rhs)
{
    return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}

inline bool operator==(Timestamp lhs, Timestamp rhs)
{
    return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
}

///
/// Gets time difference of two timestamps, result in seconds.
///
/// @param high, low
/// @return (high-low) in seconds
/// @c double has 52-bit precision, enough for one-microsecond
/// resolution for next 100 years.
inline double timeDifference(Timestamp high, Timestamp low)
{
    int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
    return static_cast<double>(diff) / Timestamp::kMicroSecondsPerSecond;
}

///
/// Add @c seconds to given timestamp.
///
/// @return timestamp+seconds as Timestamp
///
inline Timestamp addTime(Timestamp timestamp, double seconds)
{
    int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
    return Timestamp(timestamp.microSecondsSinceEpoch() + delta);
}

} // namespace netflow::base




#if 0
#include <chrono>
#include <ctime>
#include <string>

namespace netflow::base {
/*!
 * \brief 封装 chrono， C++11 chrono缺失部分使用ctime补齐，预留C++20接口 */
class Timestamp {
public:
    Timestamp() noexcept = default;
    explicit Timestamp(timespec time)
        : time_(time)
    {}
    Timestamp(Timestamp const &) noexcept = default;
    Timestamp(Timestamp &&) noexcept = default;
    /** Assigns a time stamp */
    auto operator=(Timestamp const &) &noexcept -> Timestamp & = default;
    /** Move operator */
    auto operator=(Timestamp &&) &noexcept -> Timestamp & = default;

    ~Timestamp() noexcept = default;

    auto getOnlyNanoseconds() const noexcept -> std::chrono::nanoseconds
    { return std::chrono::nanoseconds{time_.tv_nsec}; }
    auto getOnlySeconds() const noexcept -> std::chrono::seconds
    { return std::chrono::seconds {time_.tv_sec}; }

    int64_t getNanoseconds();

    static Timestamp now();
    std::string toFormattedString(bool isShowNanosecond);
private:
    timespec time_{};
    static const int kNanoSecondsPerSecond = 1000 * 1000 * 1000;

};

inline bool operator<(Timestamp lhs, Timestamp rhs) {
    return lhs.getNanoseconds() < rhs.getNanoseconds();
}
}  // namespace netflow::base
#endif


#endif //TINYNETFLOW_TIMESTAMP_H
