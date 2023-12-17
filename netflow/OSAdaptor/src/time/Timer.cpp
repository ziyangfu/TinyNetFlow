
#include "Timer.h"

using namespace netflow::base;
using namespace netflow::net;

static std::atomic_int64_t s_numCreated_ = 0;

/** cb : 定时器回调函数
 * when： 到什么时候为止时的时间戳， 触发闹钟
 * interval： 周期性闹钟，间隔时间为多少 */
 Timer::Timer(netflow::net::TimerCallback cb, netflow::base::Timestamp when, double interval)
         : callback_(cb),
           expiration_(when),
           interval_(interval),
           repeat_(interval > 0.0),
           sequence_(++::s_numCreated_)
 {
 }

/*!
 * \brief 重启定时器，即将到期时间的时间戳设置为 now + interval 后的时间戳
 * */
void Timer::restart(netflow::base::Timestamp now) {
    if (repeat_) {
       expiration_ = addTime(now, interval_);
    }
    else {
        expiration_ = Timestamp::invalid();
    }
}