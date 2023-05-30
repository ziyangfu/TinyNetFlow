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

    Channel(EventLoop* loop, int fd);
    ~Channel();
    /** 真正设置 epoll */
    void addChannel();
    void removeChannel();
    void modifyChannel();
    int getFd() const { return fd_; }
    int getEvents() const { return events_; }
    void setEvents(int event) { events_ = event; }
    // bool isNoneEvent() const { /* pass */ }

    EventLoop* getOwnerLoop() const { return loop_; }

    void handleEvent();
    /** 设置四种回调函数 */
    void setReadCallback(ReadEventCallback cb)
    { readCallback_ = std::move(cb); }
    void setWriteCallback(EventCallback cb)
    { writeCallback_ = std::move(cb); }
    void setCloseCallback(EventCallback cb)
    { closeCallback_ = std::move(cb); }
    void setErrorCallback(EventCallback cb)
    { errorCallback_ = std::move(cb); }
    /** epoll 设置IO多路复用 */
    void enableReading();  /** events_ = 3 */
    void disableReading(); /** events_ = ? */
    void enableWriting();  /** events_ = 4 */
    void disableWriting(); /** events_ = ? */
    void disableAll();     /** events_ = ? */
    bool isWriting();
    bool isReading();

private:
    void handleEventCallback();
private:
    EventLoop* loop_;
    const int fd_;
    int events_;   /** epoll 监控事件 */
    int activeEvents_; /** 活动的事件 */
    /** 四种回调函数 */
    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;

    int index_;
    std::weak_ptr<void> tie_;  /** 通过 weak_ptr转为shared_ptr的方式，判断 TcpConnection是否还存在 */
    bool tied_;
};

} // namespace netflow::net

#endif //TINYNETFLOW_CHANNEL_H
