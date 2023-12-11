//
// Created by fzy on 23-6-13.
//
#include "../../base/Logging.h"
#include "netflow/OSLayer/net/Channel.h"
#include "netflow/OSLayer/net/EventLoop.h"

#include <functional>
#include <map>
#include <thread>

#include <stdio.h>
#include <unistd.h>
#include <sys/timerfd.h>
#include <strings.h>

using namespace netflow::base;
using namespace netflow::net;
using namespace std;

void print(const char* msg)
{
    static std::map<const char*, Timestamp> lasts;
    Timestamp& last = lasts[msg];
    Timestamp now = Timestamp::now();
    //printf("%s tid %lu %s delay %f\n", now.toString().c_str(), this_thread::get_id(),
     //      msg, timeDifference(now, last));
    STREAM_INFO << " tid: " << this_thread::get_id() << " " << msg << " delay(s): "
                << timeDifference(now, last);
    last = now;
}

namespace netflow
{
    namespace net
    {
        namespace detail
        {
            int createTimerfd();
            void readTimerfd(int timerfd, Timestamp now);
        }
    }
}

// Use relative time, immunized to wall clock changes.
class PeriodicTimer
{
public:
    PeriodicTimer(EventLoop* loop, double interval, const TimerCallback& cb)
            : loop_(loop),
              timerfd_(netflow::net::detail::createTimerfd()),
              timerfdChannel_(loop, timerfd_),
              interval_(interval),
              cb_(cb)
    {
        timerfdChannel_.setReadCallback(
                std::bind(&PeriodicTimer::handleRead, this));
        timerfdChannel_.enableReading();  /** 加入epoll与channels_ */
    }

    void start()
    {
        struct itimerspec spec;
        bzero(&spec, sizeof spec);
        spec.it_interval = toTimeSpec(interval_);
        spec.it_value = spec.it_interval;
        int ret = ::timerfd_settime(timerfd_, 0 /* relative timer */, &spec, nullptr);
        if (ret)
        {
            STREAM_ERROR << "timerfd_settime()";
        }
    }

    ~PeriodicTimer()
    {
        timerfdChannel_.disableAll();
        timerfdChannel_.removeChannel();
        ::close(timerfd_);
    }

private:
    void handleRead()
    {
        loop_->assertInLoopThread();
        netflow::net::detail::readTimerfd(timerfd_, Timestamp::now());
        if (cb_)
            cb_();
    }

    static struct timespec toTimeSpec(double seconds)
    {
        struct timespec ts;
        bzero(&ts, sizeof ts);
        const int64_t kNanoSecondsPerSecond = 1000000000;
        const int kMinInterval = 100000;
        int64_t nanoseconds = static_cast<int64_t>(seconds * kNanoSecondsPerSecond);
        if (nanoseconds < kMinInterval)
            nanoseconds = kMinInterval;
        ts.tv_sec = static_cast<time_t>(nanoseconds / kNanoSecondsPerSecond);
        ts.tv_nsec = static_cast<long>(nanoseconds % kNanoSecondsPerSecond);
        return ts;
    }

    EventLoop* loop_;
    const int timerfd_;
    Channel timerfdChannel_;
    const double interval_; // in seconds
    TimerCallback cb_;
};

int main(int argc, char* argv[])
{
    //Logger::get().set_level(spdlog::level::info);
    STREAM_INFO << "pid = " << getpid() << ", tid = " << this_thread::get_id()
                 << " Try adjusting the wall clock, see what happens.";

    EventLoop loop;
    PeriodicTimer timer(&loop, 1, std::bind(print, "PeriodicTimer"));
    timer.start();
    loop.runEvery(1, std::bind(print, "EventLoop::runEvery"));
    loop.loop();
}
