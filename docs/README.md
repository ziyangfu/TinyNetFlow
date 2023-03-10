## 知识积累

畅怀未来
我认为将来有资格坐上这个宝座的网络库有两种：
1. 基于c++11标准高仿java netty的reactor模型网络库；
2. 基于c++20标准的协程网络库；

理由如下：

c++ modern network library必定要使用智能指针管理连接上下文，使用lambda设置连接断链回调、数据读写回调；
proactor模型难用，除了windows IOCP外，其它如select、poll、epoll、kqueue等IO多路复用机制都更契合reactor模型；
java同为面向对象的静态语言，java能实现的，c++必定也能实现，模仿实现netty是可行的。
实际上facebook的wangle就是在模仿netty，只是依赖了自家的folly，folly又依赖了boost，而且实现不完整；
鉴于c++20标准太新，c++20协程还有待推广挖掘，很多老项目无法立马迁移过来，所以我觉得当前最好的解决方案还是基于c++11出一个高仿netty的reactor模型网络库；
————————————————
版权声明：本文为CSDN博主「ithewei」的原创文章，遵循CC 4.0 BY-SA版权协议，转载请附上原文出处链接及本声明。
原文链接：https://blog.csdn.net/GG_SiMiDa/article/details/114804999
