//
// Created by fzy on 23-3-13.
//

#ifndef LIBZV_TIMER_ID_H
#define LIBZV_TIMER_ID_H

#include "../base/copyable.h"

namespace muduo {
namespace net {

class Timer;

class TimerId : public Copyable {
public:
    TimerId()
        : timer_(NULL),
          sequence_(0)
    {}
    ~TimerId();
    friend class TimeQueue;
private:
    Timer* timer_;
    int64_t sequence_;
};

}
}
class timer_id {

};


#endif //LIBZV_TIMER_ID_H
