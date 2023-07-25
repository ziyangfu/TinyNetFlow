# TinyNetFlow
### 1. 简述

一款轻量级开源车载领域通信中间件。底层采用事件驱动与IO多路复用方法。其具有以下特点：
- C++17标准
- Reactor网络模型
- 不跨平台，只适用于 Linux
- I/O多路复用仅采用 Linux epoll
- 仅 IPv4
- TCP服务端/客户端
- TCP支持心跳、重连、转发
- 采用三种常见的TCP拆包模式（固定包长、分界符及头部字段长度）
- 支持智能网联汽车中常用的MQTT协议、HTTP协议以及车内网络SOME/IP协议

### 2. 开发目的

充分发挥C++程序员重复造轮子的精神。哈哈。

- 现代C++编程
- Linux网络编程
- 理解内核 -> OS -> 中间件脉络

### 3. TODO
- [x] Reactor网络模型搭建
- [x] TCP server && client
- [ ] UDP server && client
- [ ] MQTT protocol
- [ ] HTTP protocol
- [ ] SOME/IP, SOME/IP-SD
- [ ] 使用异步编程promise&&future，参考seastar

### 4. 构建

CMake构建方式

```bash
mkdir build && cd build
cmake ..
make
# option
sudo make install
```

### 5. 致谢
- 底层网络库深度参考了 muduo
- MQTT代码参考了 libhv
- SOME/IP将参考 vsomeip
- 某车载领域知名国际中间件厂商

中间件底层网络库充分模仿（抄袭）了 libhv、muduo、evpp及workflow。特此感谢。

