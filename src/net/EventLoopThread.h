//
// Created by fzy on 23-5-16.
//

#ifndef TINYNETFLOW_EVENTLOOPTHREAD_H
#define TINYNETFLOW_EVENTLOOPTHREAD_H

#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <memory>
#include <string>

namespace netflow::net {

class EventLoop;

class EventLoopThread {
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;
    EventLoopThread(const std::string& name,
                    const ThreadInitCallback& cb = ThreadInitCallback());
    ~EventLoopThread();
    EventLoop* startLoop();
private:
    void threadFunc();
private:
    EventLoop* loop_;
    bool exiting_;
    bool ready_;
    std::mutex mutex_;
    std::shared_ptr<std::thread> thread_ = nullptr;
    std::condition_variable cond_;
    ThreadInitCallback callback_;

};
}  // namespace netflow::net
/** one loop per thread */



#endif //TINYNETFLOW_EVENTLOOPTHREAD_H
