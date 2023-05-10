//
// Created by fzy on 23-5-8.
//

#ifndef TINYNETFLOW_CHANNEL_H
#define TINYNETFLOW_CHANNEL_H

#include <memory>
#include <functional>

namespace netflow::net {

class EventLoop;
 /** 封装 IO 事件与回调 */
class Channel {
public:
    using ReadEventCallback = std::function<void ()>;
    using EventCallback = std::function<void ()>;
    Channel();
    ~Channel();

    void handleEvent(Timestamp receiveTime);
    void setReadCallback(ReadEventCallback cb)
    { readCallback_ = std::move(cb); }
    void setWriteCallback(EventCallback cb)
    { writeCallback_ = std::move(cb); }
    void setCloseCallback(EventCallback cb)
    { closeCallback_ = std::move(cb); }
    void setErrorCallback(EventCallback cb)
    { errorCallback_ = std::move(cb); }

    void enableReading();
    void disableReading();
    void enableWriting();
    void disableWriting();
    void disableAll();
    bool isWriting();
    bool isReading();

    void getFd();

private:
    EventLoop* loop_;
    const int fd_;
    int events_;
    int activeEvents_;
    int index_;

    std::weak_ptr<void> tie_;
    bool tied_;


};

} // namespace netflow::net

#endif //TINYNETFLOW_CHANNEL_H
