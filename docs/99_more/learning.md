- [ ] 中间件C++语言特性部分优化
> StringPiece 替换为 string_view（C++17）
目前用的是 std::string
>
>[bRPC小课堂：从StringPiece说开来](https://zhuanlan.zhihu.com/p/98829229)
>
> [string_View理解与用法（一）](https://blog.csdn.net/danshiming/article/details/122573151)
>
> [详解C++17下的string_view](https://blog.csdn.net/danshiming/article/details/116734954)
> 
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


[服务发现-从原理到实现
](https://www.cnblogs.com/gaoxingnjiagoutansuo/p/15419824.html)

[深入浅出：SOME/IP协议中的服务发现机制解析](https://blog.csdn.net/qq_21438461/article/details/134432381)


关于源文件的组织形式：
是include和src单独放的，还是放在一个文件夹下好？
有一个建议的解决思路是这样的：
private API .h文件和.cpp文件放在一个文件夹src下
public API：.h文件放在include文件夹，.cpp文件放在src文件夹下




TODO:
1. OSAdaptor
   2. DAG有向无环图，任务调度与编排
   3. 共享内存buffer的设计，参考shmipc的复杂buffer
   4. 信号的封装
   5. 协程的封装，参考 cyberRT
2. com
   1. http，参考workflow
   2. someip集成
   3. mqtt集成，修改
   4. DDS集成
5. em
   6. 任务调度，进程创建，状态管理

如何实现类似于分布式软总线的跨端通信框架
服务发现，设备发现