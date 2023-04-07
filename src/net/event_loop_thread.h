//
// Created by fzy on 23-3-13.
//

/** 将Eventloop与thread绑定在一起 */
#ifndef LIBZV_EVENT_LOOP_THREAD_H
#define LIBZV_EVENT_LOOP_THREAD_H

#include "../base/condition.h"
#include "../base/mutex.h"
#include "../base/thread.h"

namespace muduo::net {

class EventLoop;
class EventLoopThread : Noncopyable {
public:
    using ThreadInitCallback = std::function<void (EventLoop*)>;
    EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(),
                    const string& name = string());
    ~EventLoopThread();
    EventLoop* startLoop();
private:
    void threadFunc();
private:
    EventLoop* loop_ GUARDED_BY(mutex_);
    MutexLock mutex_;
    Thread thread_;
    Condition cond_ GUARDED_BY(mutex_);
    ThreadInitCallback callback_;
    bool exiting_;
};
} // namespace muduo::net

#endif //LIBZV_EVENT_LOOP_THREAD_H
