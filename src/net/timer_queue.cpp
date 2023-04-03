//
// Created by fzy on 23-3-13.
//

#include "timer_queue.h"

#include "../base/logging.h"
#include "event_loop.h"
#include "timer.h"
#include "timer_id.h"

/*!
 <sys/timerfd.h>是一个C标准库头文件，它定义了Linux系统上用于创建和操作计时器文件描述符的相关函数和常量。
 具体来说，这个头文件中包含的函数timerfd_create()用于创建一个计时器文件描述符，timerfd_settime()
 用于设置计时器的起始时间和定时器的超时时间，timerfd_gettime()用于获取计时器的当前时间和超时时间，
 以及close()用于关闭计时器文件描述符等。
 在Linux系统编程中，通过使用timerfd机制，可以很方便地实现各种计时器相关的功能，如延时等待、周期性任务调度等。*/
#include <sys/timerfd.h>
#include <unistd.h>

namespace muduo {
namespace net {
namespace detail {

int createTimerfd() {

}

struct timespec howMuchTimeFromNow(Timestamp when) {

}

void readTimerfd(int timerfd, Timestamp now) {

}

void resetTimerfd(int timerfd, Timestamp expiration) {

}

} // namespace detail
} // namespace net
}  // namespace muduo
//! -------------------------------------------------------------------------------------------
using namespace muduo;
using namespace muduo::net;
using namespace muduo::net::detail;

TimerQueue::TimerQueue(muduo::net::EventLoop *loop)
    : loop_(loop),
      timerfd_(createTimerfd()),
      timerfdChannel_(loop, timerfd_),
      timers_(),
      callingExpiredTimers_(false)
{
    timerfdChannel_.setReadCallback(std::bind(&TimerQueue::handleRead, this));  //! TODO 啥意思？？？
    timerfdChannel_.enableReading();
}

TimerQueue::~TimerQueue() {
    timerfdChannel_.disableAll();
    timerfdChannel_.remove();
    close(timerfd_);
    for (const Entry& timer : timers_) {
        delete timer.second;
    }

}

TimerId TimerQueue::addTimer(muduo::net::TimerCallback cb, muduo::Timestamp when, double interval) {

}

void TimerQueue::cancel(muduo::net::TimerId timerId) {

}

void TimerQueue::addTimerInLoop(muduo::net::Timer *timer) {


}

void TimerQueue::cancelInLoop(muduo::net::TimerId timerId) {

}

void TimerQueue::handleRead() {

}

std::vector <Entry> TimerQueue::getExpired(muduo::Timestamp now) {

}

void TimerQueue::reset(const std::vector <Entry> &expired, muduo::Timestamp now) {


}

bool TimerQueue::insert(muduo::net::Timer *timer) {

}