//
// Created by fzy on 23-3-10.
//

/*! 事件循环 */
#ifndef LIBZV_EVENT_LOOP_H
#define LIBZV_EVENT_LOOP_H

#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <memory>
#include <unordered_map>

namespace libzv {

class EventLoop {
public:
    /*! 枚举类 读事件、写事件、错误事件、水平触发方式*/

    using PollEventCB = std::function<void(int event)>;

    /*! 添加事件 */
    int AddEvent(int fd; int event, PollEventCB cb);
    /*! 删除事件 */
    int DelEvent(int fd; PollEventCB cb = nullptr);
    /*! 修改监听事件类型 */
    int ModifyEvent(int fd; int event);

    /*! 异步执行任务 */


    /*! 获取线程ID */

    /*! 获取线程名 */
private:
    /*! 阻塞当前线程 */

    /*! 结束事件轮询 */



};

} // libzv

#endif //LIBZV_EVENT_LOOP_H
