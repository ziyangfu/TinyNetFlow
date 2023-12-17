//
// Created by fzy on 23-5-31.
//

#ifndef TINYNETFLOW_TIMERQUEUE_H
#define TINYNETFLOW_TIMERQUEUE_H

#include <set>  /** 红黑树管理，自动排序 */
#include <vector>
#include <atomic>
#include <memory>

#include "Timestamp.h"
#include "netflow/OSAdaptor/include/IO/net/Callbacks.h"
#include "netflow/OSAdaptor/include/IO/reactor/Channel.h"

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
    /** Timestamp + Timer： 保证唯一性， 因为可能有同一时间到期的Timer，但是 Timer*这个地址是不可能重复的
     * 比较大小时， Timestamp中有 < 比较函数 */
    using Entry = std::pair<Timestamp, Timer*>;  /** 也可以用元组 tuple */
    /** TimerList用set而不是map的原因是。这里只有key，没有 value
     * vector adaptive AUTOSAR中用的堆， 复杂度 O(logN), 这里set为红黑树*/
    using TimerList = std::set<Entry>;
    using ActiveTimer = std::pair<Timer*, int64_t>;
    using ActiveTimerSet = std::set<ActiveTimer>;
    /** IO线程的事情，交给IO线程做 */
    void addTimerInLoop(Timer* timer);
    void cancelInLoop(TimerId timerId);
    /** 到期处理 */
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
