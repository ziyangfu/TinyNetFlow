//
// Created by fzy on 23-5-31.
//

#include "TimerQueue.h"

#include "EventLoop.h"
#include "Timer.h"
#include "TimerId.h"
#include "../base/Logging.h"

#include <sys/timerfd.h>
#include <unistd.h>

namespace netflow::net::detail {

int createTimerfd() {

}

void howMuchTimeFromNow(Timestamp now) {}

void readTimerfd(int timerfd, Timestamp now) {}

void resetTimerfd(int timerfd, Timestamp expiration) {}

}  // namespace netflow::net::detail

using namespace netflow::net::detail;
using namespace netflow::net;

TimerQueue::TimerQueue(netflow::net::EventLoop *loop)
    : loop_(loop),
      timerfd_(createTimerfd()),
      timerfdChannel_(loop, timerfd_),
      timers_(),
      callingExpiredTimers_(false)
{
    timerfdChannel_.setReadCallback(std::bind(&TimerQueue::handleRead, this));
    timerfdChannel_.enableReading();
}

TimerQueue::~TimerQueue() {
    timerfdChannel_.disableAll();
}

TimerId TimerQueue::addTimer(netflow::net::TimerCallback cb, netflow::base::Timestamp when, double interval) {

}

void TimerQueue::cancel(netflow::net::TimerId timerId) {

}

void TimerQueue::addTimerInLoop(netflow::net::Timer *timer) {

}

void TimerQueue::cancelInLoop(netflow::net::TimerId timerId) {

}

void TimerQueue::handleRead() {

}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(netflow::base::Timestamp now) {

}

void TimerQueue::reset(const std::vector<Entry> &expired, netflow::base::Timestamp now) {

}

bool TimerQueue::insert(netflow::net::Timer *timer) {

}
