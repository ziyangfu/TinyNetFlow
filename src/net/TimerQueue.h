//
// Created by fzy on 23-5-31.
//

#ifndef TINYNETFLOW_TIMERQUEUE_H
#define TINYNETFLOW_TIMERQUEUE_H

#include <set>  /** 红黑树管理，自动排序 */
#include <vector>
#include <atomic>
#include <memory>

#include "../base/Timestamp.h"
#include "Callbacks.h"
#include "Channel.h"

using namespace netflow::base;

namespace netflow::net {

class EventLoop;
class Timer;
class TimerId;

class TimerQueue {
public:
    explicit TimerQueue(EventLoop* loop);
    ~TimerQueue();

    TimerId addTimer(TimerCallback cb, Timestamp when, double interval);

    void cancel(TimerId timerId);
private:
    using Entry = std::pair<Timestamp, Timer*>;
    using TimerList = std::set<Entry>;
    using ActiveEntry = std::pair<Timer*, int64_t>;
    using ActiveTimerSet = std::set<ActiveEntry>;

    void addTimerInLoop(Timer* timer);
    void cancelInLoop(TimerId timerId);

    void handleRead();

    std::vector<Entry> getExpired(Timestamp now);

    void reset(const std::vector<Entry>& expired, Timestamp now);

    bool insert(Timer* timer);

private:
    EventLoop* loop_;
    const int timerfd_;
    Channel timerfdChannel_;

    TimerList timers_;
    ActiveTimerSet activeTimers_;
    std::atomic_bool callingExpiredTimers_;

    ActiveTimerSet cancelingTimers_;
};
} // namespace netflow::net

#endif //TINYNETFLOW_TIMERQUEUE_H
