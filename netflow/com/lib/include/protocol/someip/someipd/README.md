参考vsomeip与某中间件做法，
一个独立的应用 someipd 或者 routed，负责设备的发布订阅与消息转发，
需要实现跨设备间的SOME/IP与SOME/IP-SD通信，以及设备内应用间的
unix 域套接字通信。