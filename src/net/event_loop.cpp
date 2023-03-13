//
// Created by fzy on 23-3-10.
//

#include "event_loop.h"


#include <algorithm>
#include <signal.h>
#include <sys/eventfd.h>
#include <unistd.h>

namespace libzv::net {

EventLoop::EventLoop():
 loop_(false),
 quit_(false),
 eventHandling_(false),
 callingPendingFunctors_(false),
 iteration_(0)
{
    // pass
}

EventLoop::~EventLoop() {
    loop_ = true;
    quit_ = true;
    //pass
}
void EventLoop::loop() {

}

void EventLoop::quit() {
    quit_ = true;
    if (!isInLoopThread()) {
        wakeup();
    }
}


void EventLoop::runInLoop(Functor cb) {


}




} // libzv::net