# TODO Lists
V0.1
- [X] 时间戳与定时器
- [X] epoll处理， add，mod，del
- [X] 单元测试
- [X] 功能测试， examples， echo，chat
---
V0.2
- [X] HTTP协议移植，参考muduo http，以及其他网络库

V0.3
- [X] MQTT协议移植， 参考libhv以及其他mqtt客户端
- [X] UDP 服务端与客户端
- [X] UDP multicast部分
- [X] Unix domain socket IPC
- [ ] SOME/IP及SOME/IP-SD协议移植， 参考vsomeip

- will be release beta version (first version)
---
V1.0
- [ ] 修改与优化网络库，如采用 string_view lambda等
- [ ] 集成KCP， 若SOME/IP使用UDP协议，则走KCP协议，降低延迟
- [ ] 更多使用chrono
- [ ] 唤醒机制，除了目前的eventfd，可尝试socketpair唤醒，了解管道唤醒

---
V2.0
- [ ] 深度修改架构，将底层网络库修改为操作系统抽象层，并扩展基于共享内存的IPC。
- [ ] 增加Reactor多进程模型分支
- [ ] 移植 fastDDS
- [ ] 建立通信抽象层，底层适配SOME/IP与DDS
- [ ] 采用AUTOSAR C++14标准，朝功能安全与信息安全方向靠近，目前商业化的静态代码测试工具可以支持，但前端的代码提示与纠错工具还没有
---
V3.0
- [ ] future && promise 异步编程
- [ ] 协程，参考 CyberRT
- [ ] 使用XDP/DPDK等kernel bypass技术，聚焦实时性、确定性延迟等方向，优化网络性能
- [ ] 使用基于有向无环图的任务调度与编排框架
---
V4.0
- [ ] 执行管理与确定性调度，用户空间的执行管理，深入内核的确定性调度策略，
      调整内核调度器策略，考虑增加时间触发机制
- [ ] 

---
BPF的应用：
- [ ] 在中间件的多个关键节点写入静态的USDT（Userland Statically Defined Tracepoints），方便进行跟踪