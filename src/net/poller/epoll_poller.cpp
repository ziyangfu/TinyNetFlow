//
// Created by fzy on 23-3-10.
//

/*! 功能 ： epoll 封装 */
#include "epoll_poller.h"
#include "../../base/logging.h"
#include "../channel.h"

#include <assert.h>
#include <errno.h>
#include <poll.h>
#include <sys/epoll.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

// On Linux, the constants of poll(2) and epoll(4)
// are expected to be the same.
static_assert(EPOLLIN == POLLIN,        "epoll uses same flag values as poll");
static_assert(EPOLLPRI == POLLPRI,      "epoll uses same flag values as poll");
static_assert(EPOLLOUT == POLLOUT,      "epoll uses same flag values as poll");
static_assert(EPOLLRDHUP == POLLRDHUP,  "epoll uses same flag values as poll");
static_assert(EPOLLERR == POLLERR,      "epoll uses same flag values as poll");
static_assert(EPOLLHUP == POLLHUP,      "epoll uses same flag values as poll");

//! 匿名 namespace
namespace {
    const int kNew = -1;
    const int kAdded = 1;
    const int kDeleted = 2;
}
/*! EPOLL_CLOEXEC选项是为了保证在程序调用exec函数时，epoll实例会被自动关闭，从而避免了在子进程中不必要的资源泄漏 */
EpollPoller::EpollPoller(muduo::net::EventLoop *loop)
    : Poller(loop),
    epollfd_(epoll_create1(EPOLL_CLOEXEC)),
    events_(kInitEventListSize)  //! 初始化 vector 容器容量为 kInitEventListSize
{
    if (epollfd_ < 0) {
        LOG_SYSFATAL << "EPollPoller::EPollPoller";
    }
}

EpollPoller::~EpollPoller() {
    close(epollfd_);
}

void EpollPoller::fillActiveChannels(int numEvents, muduo::net::Poller::ChannelList *activeChannels) const {
    assert(implicit_cast<size_t>(numEvents) <= events_.size());
    for (int i = 0; i < numEvents; ++i) {
        /*! ChatGPT:
         * 在这段代码中，static_cast用于将void指针转换为Channel指针，因为events_数组中存储的是void指针，而我们需要
         * 使用的是Channel指针。这里的转换是安全的，因为我们在存储void指针的时候就已经知道这些指针实际上是指向Channel对象的。
         * 这个过程称为向下转型。需要注意的是，向下转型时需要保证转型后的指针是指向有效对象的，否则会产生未定义行为。*/
        Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
#ifndef NDEBUG
        int fd = channel->fd();
        ChannelMap::const_iterator it = channels_.find(fd);
        assert(it != channels_.end());
        assert(it->second == channel);
#endif
        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel);
    }
}

Timestamp EpollPoller::poll(int timeoutMs, muduo::net::Poller::ChannelList *activeChannels) {
    LOG_TRACE << "fd total count " << channels_.size();
    int numEvents = ::epoll_wait(epollfd_, &(*events_.begin()),
                                 static_cast<int>(events_.size()), timeoutMs);
    int savedErrno = errno;
    Timestamp now(Timestamp::now());
    if (numEvents > 0) {
        LOG_TRACE << numEvents << " events happened";
        fillActiveChannels(numEvents, activeChannels);
        if (implicit_cast<size_t>(numEvents) == events_.size()) {
            events_.resize(events_.size() * 2);
        }
    }
    else if (numEvents == 0) {
        LOG_TRACE << "nothing happened";
    }
    else {
        // error happens, log uncommon ones
        if (savedErrno != EINTR)
        {
            errno = savedErrno;
            LOG_SYSERR << "EPollPoller::poll()";
        }
    }
    return now;
}

void EpollPoller::updateChannel(muduo::net::Channel *channel) {
    Poller::assertInLoopThread();
    const int index = channel->index();
    LOG_TRACE << "fd = " << channel->fd()
              << " events = " << channel->events() << " index = " << index;
    if (index == kNew || index == kDeleted) {
        // a new one, add with EPOLL_CTL_ADD
        int fd = channel->fd();
        if (index == kNew) {
            assert(channels_.find(fd) == channels_.end());   //! map容器中没有
            channels_[fd] = channel;    //! 把 channel 放进 map
        }
        else { // index == kDeleted
            assert(channels_.find(fd) != channels_.end());  //! 找到了
            assert(channels_[fd] == channel);
        }
        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    }
    else {
        // update existing one with EPOLL_CTL_MOD/DEL
        int fd = channel->fd();
        (void)fd;   //! 这个意义是？？
        assert(channels_.find(fd) != channels_.end());
        assert(channels_[fd] == channel);
        assert(index == kAdded);
        if (channel->isNoneEvent())
        {
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        }
        else
        {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EpollPoller::removeChannel(muduo::net::Channel *channel) {
    Poller::assertInLoopThread();
    int fd = channel->fd();
    LOG_TRACE << "fd = " << fd;
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(channel->isNoneEvent());
    int index = channel->index();
    assert(index == kAdded || index == kDeleted);
    size_t n = channels_.erase(fd);
    (void)n;
    assert(n == 1);

    if (index == kAdded)
    {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew);
}

void EpollPoller::update(int operation, muduo::net::Channel *channel) {
    struct epoll_event event;
    memZero(&event, sizeof(event));
    event.events = channel->events();
    event.data.ptr = channel;
    int fd = channel->fd();
    LOG_TRACE << "epoll_ctl op = " << operationToString(operation)
              << " fd = " << fd << " event = { " << channel->eventsToString() << " }";
    if (::epoll_ctl(epollfd_, operation, fd, &event) < 0) {
        if (operation == EPOLL_CTL_DEL) {
            LOG_SYSERR << "epoll_ctl op =" << operationToString(operation) << " fd =" << fd;
        }
        else {
            LOG_SYSFATAL << "epoll_ctl op =" << operationToString(operation) << " fd =" << fd;
        }
    }
}

const char *EpollPoller::operationToString(int op) {
    switch (op)
    {
        case EPOLL_CTL_ADD:
            return "ADD";
        case EPOLL_CTL_DEL:
            return "DEL";
        case EPOLL_CTL_MOD:
            return "MOD";
        default:
            assert(false && "ERROR op");
            return "Unknown Operation";
    }
}