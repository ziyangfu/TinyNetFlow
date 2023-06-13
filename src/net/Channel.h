//
// Created by fzy on 23-5-8.
//

#ifndef TINYNETFLOW_CHANNEL_H
#define TINYNETFLOW_CHANNEL_H

#include <memory>
#include <functional>
#include "../base/Timestamp.h"

namespace netflow::net {

class EventLoop;
 /** 封装 IO 事件与回调 */
class Channel {
public:
    using ReadEventCallback = std::function<void (netflow::base::Timestamp receiveTime)>;
    using EventCallback = std::function<void ()>;

    Channel(EventLoop* loop, int fd);
    ~Channel();

    void tie(const std::shared_ptr<void>& obj);

    int getFd() const { return fd_; }
    int getEvents() const { return events_; }
    void setEvents(int event) { activeEvents_ = event; }
    int getIndex() const { return index_; }
    void setIndex(int index) { index_ = index; }
    /** 关闭读写 */
    bool isNoneEvent() const { return events_ == kNoneEvent; }

    EventLoop* getOwnerLoop() const { return loop_; }

    void handleEvent(netflow::base::Timestamp receiveTime);
    /** 设置四种回调函数 */
    void setReadCallback(ReadEventCallback cb)
    { readCallback_ = std::move(cb); }
    void setWriteCallback(EventCallback cb)
    { writeCallback_ = std::move(cb); }
    void setCloseCallback(EventCallback cb)
    { closeCallback_ = std::move(cb); }
    void setErrorCallback(EventCallback cb)
    { errorCallback_ = std::move(cb); }
    /**
        这是经典的 Epoll 对套接字事件进行处理时应用的 IO 复用的代码，以下简要说明：

        events_ |= kReadEvent; ：表示将事件的读取属性打开。位运算符 | 执行"按位或"操作，
        将 events_ 变量原有的值与 kReadEvent 变量中的值进行按位或运算，结果会使 events_ 变量中
         kReadEvent 所对应的二进制位变为 1（表示开启权限）。
        events_ &= ~kReadEvent; ：表示将事件的读取属性关闭。位运算符 &= 执行"按位与"操作，
        将 events_ 变量原有的值与 ~kReadEvent 变量的值（即 kReadEvent 所对应的值按位取反）进行按位与运算，
        结果会将 events_ 变量中 kReadEvent 所对应的二进制位置为 0（表示关闭权限）。
        events_ |= kWriteEvent; ：表示将事件的写入属性打开。
        events_ &= ~kWriteEvent; ：表示将事件的写入属性关闭。
        events_ = kNoneEvent; ：表示将事件的所有属性关闭。可以使用一些取反操作来间接实现清空操作。
        events_ & kWriteEvent ：表示检查事件是否具有写入属性。位运算符 & 执行"按位与"操作，将 events_ 变量原有的值
        与 kWriteEvent 变量的值进行按位与运算，如果结果等于 kWriteEvent 变量的值则表示事件具有写入属性。相反：如果结
        果等于 0，则表示不具有写入属性。逻辑上，该操作也可以理解为检查 events_ 中是否有 kWriteEvent 标志位，如果有则
        返回真，否则返回假。
        events_ & kReadEvent ：表示检查事件是否具有读取属性。逻辑上，该操作与检查事件是否具有写入属性的操作类似，只是
        判断的是 events_ 中是否有 kReadEvent 标志位。
        这些位运算符常常被用来对二进制的“位”进行开关控制。其中，"按位或"、"按位与"、"按位非"与"按位异或"是四种常用的位运算符。其中：

        "按位或"就是将两个二进制数进行按位或运算，结果是将两个二进位中只要有一个为 1 的位置为 1。
        例如，13 | 11 的结果为 15（二进制表示为 11012 和 10112，结果为 11112）。
        "按位与"就是将两个二进制数进行按位与运算，结果是将两个二进位中都为 1 的位置为 1。
        例如，13 & 11 的结果为 9（二进制表示为 11012 和 10112，结果为 10012）。
        "按位非"就是将一个二进制数进行按位非运算，即将 0 变为 1，将 1 变为 0。
        例如，(~13) 的结果为 -14，因为 13 的二进制表示为 1101，按位取反后为 0010（即十进制的 2），再加上一个符号位就变成了 -14。
        "按位异或"就是将两个二进制数进行按位异或（或称为“异或”）运算，结果是将两个二进位中不相同的位置为 1，相同的位置为 0。
        例如，13 ^ 11 的结果为 6（二进制表示为 11012 和 10112，结果为 01102）。
        在这份代码中，每个标志位即代表了套接字事件中的“可读”和“可写”等属性。为了控制套接字中的事件，
        代码使用了“按位或”、“按位与”、“按位非”等操作来进行位运算，从而改变套接字事件的权限。
*/
    /** 不需要了，底层已经根据以下读写函数和index进行了设置 */
    //void addChannel();
    //void modifyChannel();
    void removeChannel();
    void enableReading() { events_ |= kReadEvent; update(); } /** events_ = 3 */
    void disableReading() { events_ &= ~kReadEvent;  update();}/** events_ = ? */
    void enableWriting() { events_ |= kWriteEvent;  update(); }  /** events_ = 4 */
    void disableWriting() {events_ &= ~kReadEvent; update(); } /** events_ = ? */
    void disableAll() { events_ = kNoneEvent; };     /** events_ = ? */
    bool isWriting() const { return events_ & kWriteEvent; }
    bool isReading() const { return events_ & kReadEvent; }

private:
    void handleEventCallback(netflow::base::Timestamp receiveTime);
    void update();
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

    int index_;  /** 确定 channel是未添加的（-1），已经添加的（1），还是已经删除的（2） */
    std::weak_ptr<void> tie_;  /** 通过 weak_ptr转为shared_ptr的方式，判断 TcpConnection是否还存在 */
    bool tied_;
    bool eventHandling_;
    bool addedToLoop_;

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

};

} // namespace netflow::net

#endif //TINYNETFLOW_CHANNEL_H
