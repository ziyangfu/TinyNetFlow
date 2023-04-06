//
// Created by fzy on 23-3-31.
//

#ifndef LIBZV_POLLER_H
#define LIBZV_POLLER_H

#include <map>
#include <vector>

#include "../base/timestamp.h"
#include "event_loop.h"

namespace muduo {
namespace net {

class Channel;
/*! IO多路复用基类 */
class Poller : Noncopyable {
public:
    Poller(EventLoop* loop); // explicit
    virtual ~Poller();

    using ChannelList = std::vector<Channel*>;

    virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels) = 0;

    virtual void updateChannel(Channel* channel) = 0;

    virtual void removeChannel(Channel* channel) = 0;

    virtual bool hasChannel(Channel* channel) const;
    //! 实现在 poller/default_poller.cpp,根据环境变量选择 poll 或 epoll
    static Poller* newDefaultPoller(EventLoop* loop);

    void assertInLoopThread() const {
        ownerLoop_->assertInLoopThread();
    }
    /*! 继承类可以使用 */
protected:

    using ChannelMap = std::map<int, Channel*>;  //! 从 fd 到 Channel* 的映射, 可以换成 hash std::unordered_map
    ChannelMap channels_;
private:
    EventLoop* ownerLoop_;
};

} // namespace net
} // namespace muduo

#endif //LIBZV_POLLER_H
