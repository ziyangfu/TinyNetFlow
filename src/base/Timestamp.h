//
// Created by fzy on 23-5-26.
//

#ifndef TINYNETFLOW_TIMESTAMP_H
#define TINYNETFLOW_TIMESTAMP_H

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



#endif //TINYNETFLOW_TIMESTAMP_H
