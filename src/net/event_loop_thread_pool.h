//
// Created by fzy on 23-3-13.
//

/*! muduo网络模型
 *  1. 单 Reactor 多线程模型
 *      1个IO线程，多个计算线程
 *  2. 多 Reactor 多线程模型
 *      多个IO线程，分为 mainReactor 和 subReactor， 多个计算线程
 *  */

#ifndef LIBZV_EVENT_LOOP_THREAD_POOL_H
#define LIBZV_EVENT_LOOP_THREAD_POOL_H

#include "../base/noncopyable.h"
#include "../base/types.h"
#include <functional>
#include <memory>
#include <vector>

namespace muduo::net {

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : Noncopyable {
public:
    using ThreadInitCallback = std::function<void (EventLoop*)>;
    EventLoopThreadPool(EventLoop* baseLoop, const string& nameArg);
    ~EventLoopThreadPool();

    void setThreadNum(int numThreads) { numThreads_ = numThreads; }
    void start(const ThreadInitCallback& cb = ThreadInitCallback());

    EventLoop* getNextLoop();

    EventLoop* getLoopForHash(size_t hashCode);

    std::vector<EventLoop*> getAllLoops();

    bool started() const { return started_; }

    const string& name() const { return name_; }
private:
    EventLoop* baseLoop_;
    string name_;
    bool started_;
    int numThreads_;
    int next_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_; /*! IO线程的线程池，线程数大于1时，即多 Reactor 模式 */
    std::vector<EventLoop*> loops_;
};

}  // namespace muduo::net

#endif //LIBZV_EVENT_LOOP_THREAD_POOL_H
