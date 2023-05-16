//
// Created by fzy on 23-5-10.
//

#include "EventLoop.h"
#include "EpollPoller.h"
#include "Channel.h"
using namespace netflow::net;

/********************************************************************************************/
/** 匿名空间 */
namespace
{
    //__thread EventLoop* t_loopInThisThread = nullptr;  /** 保存当前 EventLoop this指针 */

    const int kPollTimeMs = 10000;


/** 使用eventfd 对象实现异步唤醒功能 */
    int createEventfd()
    {
        int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        if (evtfd < 0)
        {
            abort();
        }
        return evtfd;
    }

#pragma GCC diagnostic ignored "-Wold-style-cast"
    class IgnoreSigPipe
    {
    public:
        IgnoreSigPipe()
        {
            ::signal(SIGPIPE, SIG_IGN);
            // LOG_TRACE << "Ignore SIGPIPE";
        }
    };
#pragma GCC diagnostic error "-Wold-style-cast"

    IgnoreSigPipe initObj;
}  // namespace

EventLoop* EventLoop::getEventLoopOfCurrentThread()
{
    return t_loopInThisThread;
}
/********************************************************************************************/

EventLoop::EventLoop()
    : poller_(std::make_unique<EpollPoller>(this)),
      wakeupFd_(createEventfd()),
      wakeupChannel_(std::make_unique<Channel>(this, wakeupFd_)),
      looping_(false),
      quit_(false),
      eventHandling_(false),
      callingPendingFunctors_(false),
      currentActiveChannel_(nullptr)
{
    if(m_loopInThisThread) {
        /** 之前已经创建过 EventLoop， 违背了 one loop per thread， 报错 */
    }
    else {
        m_loopInThisThread = this;  /** 保存 this 指针 */
    }
    /** 设置唤醒机制 */
    wakeupChannel_->setReadCallback([](){
        uint64_t one = 1;
        ssize_t n = sockets::read(wakeupFd_, &one, sizeof one); /** SocketsOps封装 */
        if (n != sizeof one)
        {
            /** 出错啦 */
        }
    });
    wakeupChannel_->enableReading();  /** 将event fd 纳入 epoll 监控范围 */
}

EventLoop::~EventLoop() {
    /** 移除 wakeup相关设置 */
    wakeupChannel_->disableAll()
    wakeupChannel_->removeChannel();
    ::close(wakeupFd_);
    t_loopInThisThread = nullptr;
}

void EventLoop::loop() {
    looping_ = true;
    quit_ = false;
    /** 无限循环 */
    while (!quit_){
        activeChannels_.clear();
        poller_->poll(10000,&activeChannels_);  /**将长期阻塞在这里，等待事件发生 */

        eventHandling_ = true;
        for(Channel* channel : activeChannels_) {
            currentActiveChannel_ = channel;
            currentActiveChannel_->handleEvent();  /** 处理 epoll 发生事件 */
        }
        currentActiveChannel_ = nullptr;
        eventHandling_ = false;
        doPendingFunctors();  /** 处理上层事件 */
    }
    looping_ = false;
}

void EventLoop::quit() {
    quit_ = true;
    /** IO相关的事情，在IO线程中处理 */
    if (!isInLoopThead()) {
        wakeup();
    }
}
/** EventLoop 充当 Channel 与 EpollPoller 的连接纽带 */
void EventLoop::addChannel(netflow::net::Channel *channel) {
    /** TODO 需判断，只能操作绑定本EventLoop的Channel */
    poller_->addChannel(channel);

}

void EventLoop::removeChannel(netflow::net::Channel channel) {
    poller_->removeChannel(channel);

}

void EventLoop::modifyChannel(netflow::net::Channel channel) {
    poller_->modifyChannel(channel);

}
/**
 * \brief 唤醒IO线程，即将EventLoop从loop函数的poll中唤醒，使得loop循环可以去处理 doPendingFunctors
 * */
void EventLoop::wakeup() {}

void EventLoop::handleReadForWakeup() {

}

void EventLoop::runInLoop(Functor cb) {
    if(isInLoopThead()){
        cb();
    }
    else {
        queueInLoop(std::move(cb));
    }

}

void EventLoop::queueInLoop(Functor cb) {
    std::unique_lock<std::mutex> lock(mutex_);
    pendingFunctors_.push_back(std::move(cb));
    lock.unlock();
    if((!isInLoopThead()) || callingPendingFunctors_) {
        wakeup();
    }

}
/*!
 * \brief 执行上层回调 */
void EventLoop::doPendingFunctors() {
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;
    /** 为了防止阻塞，交换 */
    std::unique_lock<std::mutex> lock(mutex_);
    functors.swap(pendingFunctors_);
    lock.unlock();
    /** 处理回调 */
    for (const Functor& functor : functors)
    {
        functor();
    }
    callingPendingFunctors_ = false;
}