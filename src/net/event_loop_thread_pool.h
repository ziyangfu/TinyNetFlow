//
// Created by fzy on 23-3-13.
//

/*! libzv 默认多线程 IO 模型 */

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
    std::vector<std::unique_ptr<EventLoopThread>> threads_; /*! 线程池 */
    std::vector<EventLoop*> loops_;
};

}  // namespace muduo::net

#endif //LIBZV_EVENT_LOOP_THREAD_POOL_H
