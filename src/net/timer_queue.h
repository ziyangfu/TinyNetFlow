//
// Created by fzy on 23-3-13.
//

#ifndef LIBZV_TIMER_QUEUE_H
#define LIBZV_TIMER_QUEUE_H

#include <set>  //! 它包含一组不重复的元素，这些元素按照其值的大小顺序进行排序。
#include <vector>

#include "../base/mutex.h"
#include "../base/timestamp.h"
#include "callback.h"
#include "channel.h"

namespace muduo {
namespace net {

class EventLoop;  //! 前向声明
class Timer;
class TimerId;

class TimerQueue : Noncopyable {
public:
    explicit TimerQueue(EventLoop* loop);
    ~TimerQueue();

    TimerId addTimer(TimerCallback cb, Timestamp when, double interval);
    void cancel(TimerId timerId);

private:
    using Entry = std::pair<Timestamp, Timer*>;
    using TimerList = std::set<Entry>;
    using ActiveTimer = std::pair<Timer*, int64_t>;
    using ActiveTimerSet = std::set<ActiveTimer>;

    void addTimerInLoop(Timer* timer);
    void cancelInLoop(TimerId timerId);
    //! 时间到期时调用
    void handleRead();
    //! 移出所有到期的定时器

    std::vector<Entry> getExpired(Timestamp now);
    void reset(const std::vector<Entry>& expired, Timestamp now);
    bool insert(Timer* timer);

private:
    EventLoop* loop_;
    const int timerfd_;
    Channel timerfdChannel_;
    //!  根据终止时间对 timer list 排序
    TimerList timers_;
    ActiveTimerSet activeTimers_;
    bool callingExpiredTimers_;
    ActiveTimerSet cancelingTimers_;
};

} // namespace net
} // namespace muduo


#endif //LIBZV_TIMER_QUEUE_H
