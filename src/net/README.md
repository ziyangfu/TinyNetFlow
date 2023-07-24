# net模块
TODO: StringPiece 替换为 string_view（C++17）
目前用的是 std::string
[bRPC小课堂：从StringPiece说开来](https://zhuanlan.zhihu.com/p/98829229)

[string_View理解与用法（一）](https://blog.csdn.net/danshiming/article/details/122573151)

[详解C++17下的string_view](https://blog.csdn.net/danshiming/article/details/116734954)

TODO:

V0.1
- [X] 时间戳与定时器
- [X] epoll处理， add，mod，del
- [X] 单元测试
- [X] 功能测试， examples， echo，chat
---
V0.2
- [X] HTTP协议移植，参考muduo http，以及其他网络库

V0.3
- [ ] MQTT协议移植， 参考libhv以及其他mqtt客户端
- [ ] SOME/IP协议移植， 参考vsomeip
---
V1.0
- [ ] 修改与优化网络库，如采用 string_view lambda等
- [ ] 定时器修改为跨平台，更多使用chrono
- [ ] 唤醒机制，除了目前的eventfd，尝试socketpair唤醒，了解管道唤醒
- [ ] 内置TCP拆包方式(三种简单方式)
- [ ] TCP 心跳
---
V2.0
- [ ] 深度修改架构，将底层网络库修改为操作系统抽象层，并扩展基于unix域套接字或共享内存的IPC。
- [ ] option: 多线程模型修改为多进程模型
- [ ] 移植 fastDDS
- [ ] 建立通信抽象层，底层适配SOME/IP与DDS
---
V3.0
- [ ] future && promise 异步编程
- [ ] 协程，参考 CyberRT
---