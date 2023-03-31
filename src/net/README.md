## libzv event事件循环实现

1. 使用智能指针管理连接上下文
2. 使用lambda设置连接断链回调、数据读写回调

EVENT_EPOLL实现

IO多路复用统一抽象接口(暂时不需要)

TCP拆包

事件循环模块对外头文件

事件结构体定义

- 网络IO事件
- 定时器事件
- 空闲事件
- 自定义事件

非阻塞IO


与网络性能相关的两个参数为poller_threads和handler_threads：
* poller线程主要负责epoll（kqueue）和消息反序列化。
* handler线程是网络任务callback和process所在线程。

net 文件编辑顺序

