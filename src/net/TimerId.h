//
// Created by fzy on 23-5-31.
//

#ifndef TINYNETFLOW_TIMERID_H
#define TINYNETFLOW_TIMERID_H

#include <cstdint>
#include <memory>
/*!
 * \brief
 * \public 公开类，用户调用
 * */

namespace netflow::net {

class Timer;

class TimerId {
public:
    TimerId(Timer* timer, int64_t seq)
        : timer_(timer),
          sequence_(seq)
    {}
    ~TimerId() = default;
    /** public 友元： 可以访问 public和private
     *  private 友元： 可以访问 public， 不能访问 private */
    friend class TimerQueue;
private:
    Timer* timer_;  /* FIXME: unique_ptr?? */
    int64_t sequence_;
};
}  // namespace netflow::net

#endif //TINYNETFLOW_TIMERID_H
