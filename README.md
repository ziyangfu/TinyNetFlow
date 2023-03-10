# libzv
### 1. 简述

一款采用事件驱动与IO多路复用的简单的C++网络库。其具有以下特点：
- C++17标准
- Reactor网络模型
- 不跨平台，只适用于 Linux
- I/O多路复用仅采用 Linux epoll
- 高性能事件循环（网络IO事件、定时器事件、空闲事件及自定义事件）
- TCP/UDP服务端/客户端
- TCP支持心跳、重连、转发
- 采用三种常见的TCP拆包模式（固定包长、分界符及头部字段长度）
- MQTT客户端（因为它最简单），后面再支持HTTP

### 2. 开发目的

充分发挥C++程序员重复造轮子的精神。哈哈。

- 学习现代C++编程
- 学习Linux网络编程
- 理解内核 -> OS -> 中间件脉络
- 学习如何开发动态/静态链接库

### 3. 构建

CMake构建方式

```bash
mkdir build && cd build
cmake ..
make
```

### 4. 致谢

本C++网络库充分模仿（抄袭）了 libev、libhv、libuv、muduo及workflow。特此感谢。

