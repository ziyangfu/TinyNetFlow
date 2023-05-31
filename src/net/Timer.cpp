//
// Created by fzy on 23-5-25.
//

#include "Timer.h"

using namespace netflow::base;
using namespace netflow::net;

void Timer::restart(netflow::base::Timestamp now) {
    if (repeat_) {
       // expiration_ = addTime(now, interval_);
    }
}