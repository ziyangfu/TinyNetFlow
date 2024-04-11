# SOME/IP && SOME/IP-SD协议实现

本协议栈参考了vsomeip的设计实现，其对外接口、部分内部设计实现以及
配置文件格式均与vsomeip保持一致。

关于SOME/IP-SD的服务发现、应用的注册等操作，均放在了SOME/IP守护进程内。
所有本机或跨机通信，均通过SOME/IP守护进程进行。SOME/IP的APP，
均采用UNIX域套接字或共享内存与SOME/IP守护进程通信，进而与外部通信。

## 基本说明

1. 不实现SOME/IP-TP协议
2. SOME/IP底层仅基于TCP实现
3. SOME/IP-SD基于UDP多播实现，最好是IPv6之上的UDP多播
4. 本机或跨设备目前仅支持网络协议栈，后续支持本机UDS与共享内存 
5. 暂时不实现 E2E 功能 
6. API接口参考了vsomeip


TODO:
- [ ] UDP 组播
- [ ] UDS IPC

- [ ] SOME/IP协议
- [ ] SOME/IP-SD协议