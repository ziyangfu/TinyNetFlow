//
// Created by fzy on 23-5-10.
//

#include "EventLoop.h"
#include "EpollPoller.h"
using namespace netflow::net;

void EventLoop::loop() {
    looping_ = true;
    quit_ = false;
    // 无限循环
    while (!quit_){
        activeChannels_.clear();
        poller_->poll(1,&activeChannels_);

        eventHandling_ = true;
        for(Channel* channel : activeChannels_) {
            currentActiveChannel_ = channel;
        }


    }

}
