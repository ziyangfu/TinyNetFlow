//
// Created by fzy on 23-3-10.
//

#include "event_loop.h"

#include "../base/logging.h"
#include "../base/mutex.h"
#include "channel.h"
#include "poller.h"
#include "socket_ops.h"
#include "timer_queue.h"

#include <algorithm>
#include <signal.h>
#include <sys/eventfd.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;
//! 匿名 namespace
//! https://blog.csdn.net/Solstice/article/details/6186978
namespace {
/** 保证一个线程只创建一个Eventloop */
__thread EventLoop* t_loopInThisThread = nullptr;
/** 定义默认的poller IO复用超时时间， 10秒 */
const int kPollTimeMs = 10000;
/**
 * 使用eventfd 对象实现异步唤醒功能
 * 创建线程之后主线程和子线程谁先运行是不确定的。
 * 通过一个eventfd在线程之间传递数据的好处是多个线程无需上锁就可以实现同步。
 * eventfd支持的最低内核版本为Linux 2.6.27,在2.6.26及之前的版本也可以使用eventfd，但是flags必须设置为0。
 * 函数原型：
 *     #include <sys/eventfd.h>
 *     int eventfd(unsigned int initval, int flags);
 * 参数说明：
 *      initval,初始化计数器的值。
 *      flags, EFD_NONBLOCK,设置socket为非阻塞。
 *             EFD_CLOEXEC，执行fork的时候，在父进程中的描述符会自动关闭，子进程中的描述符保留。
 * 场景：
 *     eventfd可以用于同一个进程之中的线程之间的通信。
 *     eventfd还可以用于同亲缘关系的进程之间的通信。
 *     eventfd用于不同亲缘关系的进程之间通信的话需要把eventfd放在几个进程共享的共享内存中（没有测试过）。
 */
//! 创建wakeupfd 用来notify唤醒subReactor处理新来的channel
int createEventfd() {
    int evtfd = eventfd(0,EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0) {
        LOG_SYSERR << "Failed in eventfd";
        abort();
    }
    return evtfd;
}
/** 跟编译器有关的看不懂 */
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
}  // 匿名 namespace

EventLoop* EventLoop::getEventLoopOfCurrentThread() {
    return t_loopInThisThread;
}

EventLoop::EventLoop()
    : looping_(false),
      quit_(false),
      eventHandling_(false),
      callingPendingFunctors_(false),
      iteration_(0),
      threadId_(CurrentThread::tid()),
      poller_(Poller::newDefaultPoller(this)),
      timerQueue_(std::make_unique<TimerQueue>(this)),    // timerQueue_(new TimerQueue(this)),
      wakeupFd_(createEventfd()),
      wakeupChannel_(std::make_unique<Channel>(this, wakeupFd_)),
      currentActiveChannel_(NULL)
{
    LOG_DEBUG << "EventLoop created " << this << " in thread " << threadId_;
    /** 不能有值，有值代表线程已经创建了一个Eventloop，不允许重复创建 */
    if (t_loopInThisThread)
    {
        LOG_FATAL << "Another EventLoop " << t_loopInThisThread
                  << " exists in this thread " << threadId_;
    }
    else
    {
        t_loopInThisThread = this;
    }
    /** 设置 wakeupfd 的时间类型以及发生事件后的回调操作， 读事件，handleRead函数处理 */
    wakeupChannel_->setReadCallback(
            std::bind(&EventLoop::handleRead, this));
    // we are always reading the wakeupfd
    /** 注册可读事件，每个Eventloop都将监听wakeupChannel_的EPOLL读事件 */
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop() {
    LOG_DEBUG << "EventLoop " << this << " of thread " << threadId_
              << " destructs in thread " << CurrentThread::tid();
    /** 移除 channel中所有感兴趣的事件 */
    wakeupChannel_->disableAll();
    /** 从Eventloop中删除channel */
    wakeupChannel_->remove();
    ::close(wakeupFd_);
    t_loopInThisThread = NULL;
}
/*!
 * 1. 判断是否是一个线程一个loop
 * 2. 开始循环
 * 3. activeChannels_初始化（清空），等待 poller 返回活动事件 Channel
 * 4. 根据活动Channel 处理回调事件 handleEvent
 * 5. 处理结束，继续循环 */
void EventLoop::loop() {
    assert(!looping_);
    assertInLoopThread();
    looping_ = true;
    quit_ = false;  // FIXME: what if someone calls quit() before loop() ?
    LOG_TRACE << "EventLoop " << this << " start looping";
    //! 开始循环
    while (!quit_)
    {
        activeChannels_.clear();
        pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
        ++iteration_;
        if (Logger::logLevel() <= Logger::TRACE)
        {
            printActiveChannels();
        }
        // TODO sort channel by priority
        eventHandling_ = true;
        for (Channel* channel : activeChannels_)
        {
            currentActiveChannel_ = channel;
            /** Poller监听哪些channel发生了事件 然后上报给EventLoop 通知channel处理相应的事件 */
            currentActiveChannel_->handleEvent(pollReturnTime_);
        }
        currentActiveChannel_ = NULL;
        eventHandling_ = false;
        /**
        * 执行当前EventLoop事件循环需要处理的回调操作 对于线程数 >=2 的情况 IO线程 mainloop(mainReactor) 主要工作：
        * accept接收连接 => 将accept返回的connfd打包为Channel => TcpServer::newConnection通过轮询将TcpConnection对象
         * 分配给subloop处理
        *
        * mainloop调用queueInLoop将回调加入subloop（该回调需要subloop执行 但subloop还在poller_->poll处阻塞） queueInLoop
         * 通过wakeup将subloop唤醒
        **/
        doPendingFunctors();
    }

    LOG_TRACE << "EventLoop " << this << " stop looping";
    looping_ = false;
}
/**
 * 退出事件循环
 * 1. 如果loop在自己的线程中调用quit成功了 说明当前线程已经执行完毕了loop()函数的poller_->poll并退出
 * 2. 如果不是当前EventLoop所属线程中调用quit退出EventLoop 需要唤醒EventLoop所属线程的epoll_wait
 *
 * 比如在一个subloop(worker)中调用mainloop(IO)的quit时 需要唤醒mainloop(IO)的poller_->poll 让其执行完loop()函数
 *
 * ！！！ 注意： 正常情况下 mainloop负责请求连接 将回调写入subloop中 通过生产者消费者模型即可实现线程安全的队列
 * ！！！       但是muduo通过wakeup()机制 使用eventfd创建的wakeupFd_ notify 使得mainloop和subloop之间能够进行通信
 **/
void EventLoop::quit() {
    quit_ = true;
    if (!isInLoopThread()) {
        wakeup();
    }
}
/** 在当前loop中执行cb
 * 1. 在本线程，即当前EventLoop中执行回调
 * 2. 在非当前EventLoop线程中执行cb，就需要唤醒EventLoop所在线程执行cb */
void EventLoop::runInLoop(Functor cb) {
    if (isInLoopThread())
    {
        cb();
    }
    else
    {
        queueInLoop(std::move(cb));
    }
}
/** 把cb放入队列中 唤醒loop所在的线程执行cb */
void EventLoop::queueInLoop(muduo::net::EventLoop::Functor cb) {
    {
        MutexLockGuard lock(mutex_);
        /** 将当前回调放入回调容器中 */
        pendingFunctors_.push_back(std::move(cb));
    }
    /**
       * || callingPendingFunctors的意思是 当前loop正在执行回调中 但是loop的pendingFunctors_中
       * 又加入了新的回调 需要通过wakeup写事件
       * 唤醒相应的需要执行上面回调操作的loop的线程 让loop()下一次poller_->poll()不再阻塞
       * （阻塞的话会延迟前一次新加入的回调的执行），然后继续执行pendingFunctors_中的回调函数
       **/
    if (!isInLoopThread() || callingPendingFunctors_)
    {
        /** 唤醒loop所在线程 */
        wakeup();
    }
}

size_t EventLoop::queueSize() const {
    MutexLockGuard lock(mutex_);
    return pendingFunctors_.size();
}
/*! 现在运行 */
TimerId EventLoop::runAt(muduo::Timestamp time, muduo::net::TimerCallback cb) {
    return timerQueue_->addTimer(std::move(cb), time, 0.0);
}
/*! 延迟多少秒后运行 */
TimerId EventLoop::runAfter(double delay, muduo::net::TimerCallback cb) {
    Timestamp time(addTime(Timestamp::now(), delay));
    return runAt(time, std::move(cb));
}
/*! 每隔多少秒运行 */
TimerId EventLoop::runEvery(double interval, muduo::net::TimerCallback cb) {
    Timestamp time(addTime(Timestamp::now(), interval));
    return timerQueue_->addTimer(std::move(cb), time, interval);
}

void EventLoop::cancel(muduo::net::TimerId timerId) {
    return timerQueue_->cancel(timerId);
}
/*! 调用 poller， 底层调用 epoll_ctl */
void EventLoop::updateChannel(muduo::net::Channel *channel) {
    assert(channel->ownerLoop() == this);  //! 确定 Channel 绑定的线程是当前线程
    assertInLoopThread();
    poller_->updateChannel(channel);
}
/*! 调用 poller， 底层调用 epoll_ctl */
void EventLoop::removeChannel(muduo::net::Channel *channel) {
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    //! 把 activeChannel中的 channel 删除
    if (eventHandling_)
    {
        assert(currentActiveChannel_ == channel ||
               std::find(activeChannels_.begin(), activeChannels_.end(), channel) == activeChannels_.end());
    }
    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(muduo::net::Channel *channel) {
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    return poller_->hasChannel(channel);
}

void EventLoop::abortNotInLoopThread() {
    LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
              << " was created in threadId_ = " << threadId_
              << ", current thread id = " <<  CurrentThread::tid();
}
/** 用来唤醒loop所在线程 向wakeupFd_写一个数据 wakeupChannel就发生读事件 当前loop线程就会被唤醒 */
void EventLoop::wakeup() {
    uint64_t one = 1;
    ssize_t n = sockets::write(wakeupFd_, &one, sizeof one);  //! 搞清楚 socket write
    if (n != sizeof one)
    {
        LOG_ERROR << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
    }
}

void EventLoop::handleRead() {
    uint64_t one = 1;
    ssize_t n = sockets::read(wakeupFd_, &one, sizeof one);
    if (n != sizeof one)
    {
        LOG_ERROR << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
    }
}

void EventLoop::doPendingFunctors() {
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;
    {
        MutexLockGuard lock(mutex_);
        /** 交换的方式减少了锁的临界区范围 提升效率 同时避免了死锁 如果执行functor()在临界区内
         * 且functor()中调用queueInLoop()就会产生死锁 */
        functors.swap(pendingFunctors_);
    }

    for (const Functor& functor : functors)
    {
        functor();
    }
    callingPendingFunctors_ = false;
}

void EventLoop::printActiveChannels() const {
    for (const Channel* channel : activeChannels_)
    {
        LOG_TRACE << "{" << channel->reventsToString() << "} ";
    }
}
