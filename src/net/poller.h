//
// Created by fzy on 23-3-31.
//

#ifndef LIBZV_POLLER_H
#define LIBZV_POLLER_H

#include <map>
#include <vector>

#include "../base/timestamp.h"
#include "event_loop.h"

namespace muduo {
namespace net {

class Poller : Noncopyable {
    static Poller* newDefaultPoller(EventLoop* loop);
};
} // namespace net
} // namespace muduo



#endif //LIBZV_POLLER_H
