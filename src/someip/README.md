# SOME/IP && SOME/IP-SD协议实现

## 基本说明

1. 不实现SOME/IP-TP协议
2. SOME/IP底层仅基于TCP实现
3. SOME/IP-SD基于UDP多播实现，最好是IPv6之上的UDP多播
4. 本机或跨设备目前仅支持网络协议栈，后续支持本机UDS与共享内存 
5. 暂时不实现 E2E 功能 
6. API接口参考了vsomeip

## 编程技法
采用Pimpl Idiom 技法，不使用面向对象技法