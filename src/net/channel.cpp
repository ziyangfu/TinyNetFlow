//
// Created by fzy on 23-3-13.
//

#include "channel.h"
#include "../base/logging.h"
#include "event_loop.h"

#include <sstream>
#include <poll.h>

using namespace muduo;
using namespace muduo::net;

const int Channel::kNoneEvent = 0;
/*!
#define POLLIN		0x001		There is data to read.
#define POLLPRI		0x002		There is urgent data to read.
#define POLLOUT		0x004		 Writing now will not block.
 */
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(muduo::net::EventLoop *loop, int fd)
    : loop_(loop),
      fd_(fd),
      events_(0),
      revents_(0),
      index_(-1),
      logHub_(true),
      tied_(false),
      eventHandling_(false),
      addedToLoop_(false) {

}

Channel::~Channel() {}

void Channel::tie(std::shared_ptr<void> &) {

}

void Channel::update() {

}

void Channel::remove() {

}

void Channel::handleEvent(muduo::Timestamp receiveTime) {

}

void Channel::handleEventWithGuard(muduo::Timestamp receiveTime) {

}

string Channel::reventsToString() const {

}

string Channel::eventsToString() const {

}

string Channel::eventsToString(int fd, int ev) {

}