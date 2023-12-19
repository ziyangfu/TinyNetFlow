//
// Created by fzy on 23-5-16.
//

#ifndef TINYNETFLOW_OSADAPTOR_EVENTLOOPTHREADPOOL_H
#define TINYNETFLOW_OSADAPTOR_EVENTLOOPTHREADPOOL_H

#include <functional>
#include <memory>
#include <vector>
#include <string>
namespace netflow::net {

class EventLoopThread;
class EventLoop;

class EventLoopThreadPool {
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;
    EventLoopThreadPool(EventLoop* baseLoop, const std::string& name);
    ~EventLoopThreadPool();

    void setThreadNum(int numThreads) { numThreads_ = numThreads; }
    void start(const ThreadInitCallback& cb = ThreadInitCallback());
    /** 采用RR调度 */
    EventLoop* getNextLoop();
    EventLoop* getLoopForHash(size_t hashCode);
    std::vector<EventLoop*> getAllLoops();

    bool isStarted() const { return started_; }
    const std::string& getName() const { return name_; }

private:
    EventLoop* baseLoop_;
    std::string name_;
    bool started_;
    int numThreads_; /** IO线程池中的线程数 */
    int next_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_;   /** IO线程池 */
    std::vector<EventLoop*> loops_;

};

} // namespace netflow::net



#endif //TINYNETFLOW_OSADAPTOR_EVENTLOOPTHREADPOOL_H
