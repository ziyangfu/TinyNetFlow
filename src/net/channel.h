//
// Created by fzy on 23-3-13.
//

/*!
 * 用于每个 socket 连接的事件分发
 * Channel 与 EventLoop 是 多对1 的关系
 * EventLoop 作为 Channel 与 poller 沟通的桥梁 */

#ifndef LIBZV_CHANNEL_H
#define LIBZV_CHANNEL_H

#include <memory>
#include <functional>

#include "../base/noncopyable.h"
#include "../base/timestamp.h"

namespace muduo {
namespace net {

class EventLoop;
/*! 封装 IO 事件与回调 不拥有 fd */
class Channel : Noncopyable {
public:
    using EventCallback = std::function<void ()>;
    using ReadEventCallback = std::function<void (Timestamp)>;

    Channel(EventLoop* loop, int fd);
    ~Channel();

    void handleEvent(Timestamp receiveTime);
    void setReadCallback(ReadEventCallback cb) { readCallback_ = std::move(cb); }
    void setWriteCallback(EventCallback cb) {writeCallback_ = std::move(cb); }
    void setCloseCallback(EventCallback cb) {closeCallback_ = std::move(cb); }
    void setErrorCallback(EventCallback cb) {errorCallback_ = std::move(cb); }

    void tie(std::shared_ptr<void>&);
    //! get fd
    int fd() const {return fd_; };
    int events() const {return events_; }
    void set_revents(int revt) { events_ = revt; }   //! poller 用
    bool isNoneEvent() const { return events_ == kNoneEvent; }

    void enableReading() { events_ |= kReadEvent; update(); }  //! 按位或， events_ = events_ | kReadEvent
    void disableReading() {events_ &= ~kReadEvent; update(); }  //! ~ 按位取反

    void enableWriting() { events_ |= kWriteEvent; update(); }
    void disableWriting() { events_ &= ~kWriteEvent; update(); }

    void disableAll() { events_ = kNoneEvent; update(); }
    bool isWriting() const { return events_ & kWriteEvent; }
    bool isReading() const { return events_ & kReadEvent; }

    //! poller 用
    int index() {return index_; }
    void set_index(int idx) {index_ = idx; }
    //! debug用
    string reventsToString() const;
    string eventsToString() const;

    void doNotLogHup() { logHub_ = false; }
    //! 返回 Channel 所属的线程
    EventLoop* ownerLoop() {return loop_; }

    void remove();


private:
    static string eventsToString(int fd, int ev);
    void update();
    void handleEventWithGuard(Timestamp receiveTime);
private:
    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop* loop_;
    const int fd_;
    int events_; //! channel 关心的事件
    int revents_; //! poller 返回的已经就绪的事件, 目前活动的事件
    int index_; //! 给 poller 用
    int logHub_;

    std::weak_ptr<void> tie_;
    bool tied_;
    bool eventHandling_;
    bool addedToLoop_;
    ReadEventCallback  readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;
};

} // namespace net
} // namespace muduo

#endif //LIBZV_CHANNEL_H
