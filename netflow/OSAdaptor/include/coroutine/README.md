## 有栈协程实现
### 1. 概述
采用有栈协程及共享栈的设计实现，而非C++20的无栈协程方案。整体基于腾讯的协程库libco修改适配而来。
libco主要面向服务器设计，因此CPU架构方面，主要是X86与AMD64架构。
上下文切换方面，也基于服务器的使用场景，去除了浮点与信号屏蔽掩码上下文，只对寄存器与栈顶指针进行切换。

### 2. 支持的平台
libco主要面向服务器设计，因此CPU架构方面，主要是X86与AMD64架构。
上下文切换方面，也基于服务器的使用场景，去除了浮点与信号屏蔽掩码上下文，只对寄存器与栈顶指针进行切换。
TinyNetFlow有面向嵌入式开发的需求，因此需要支持ARM64平台。X86与ARM现在用的已经不多了，可以舍弃。
因此主要支持以下平台：
- AMD64(X86-64)
- ARM64

### 3.相关库与资料

需要改造为支持ARM及ARM64架构
ARM64如果不重点关注性能，可以直接使用glibc的context

很多消息传输，因此会导致大量的IO消耗，那么·如何减少任务切换带来的开销就显得尤为重要。

库及相关解析资料：
- [腾讯 libco协程库](https://github.com/Tencent/libco)
- [云风对libco的代码注释](https://github.com/chenyahui/AnnotatedCode/tree/master/libco)
- [微信 libco 协程库源码分析](https://www.cyhone.com/articles/analysis-of-libco/)
- [云风 coroutine 协程库源码分析](https://www.cyhone.com/articles/analysis-of-cloudwu-coroutine/)
- [云风极简协程实现](https://github.com/cloudwu/coroutine/)
- [云风极度协程库的注释版](https://github.com/chenyahui/AnnotatedCode/tree/master/coroutine)
- [tinyrpc的协程](https://github.com/Gooddbird/tinyrpc/tree/main/tinyrpc/coroutine)
- [libtask:用于 C 和 Unix 的协程库](https://swtch.com/libtask/)

其他的库
- https://github.com/duanery/coroutine/tree/master
- https://github.com/kurocha/coroutine-arm64/tree/master



有栈协程的原理
一个程序要真正运行起来，需要两个因素：可执行代码段、数据。体现在 CPU 中，主要包含以下几个方面：

EIP 寄存器：用来存储 CPU 要读取指令的地址
ESP 寄存器：指向当前线程栈的栈顶位置
其他通用寄存器的内容：包括代表函数参数的 rdi、rsi 等等。
线程栈中的内存内容。
这些数据内容，我们一般将其称为 “上下文” 或者 “现场”。

有栈协程的原理，就是从线程的上下文下手，如果把线程的上下文完全改变。即：改变 EIP 寄存的内容，指向其他指令地址；改变线程栈的内存内容等等。
这样的话，当前线程运行的程序也就完全改变了，是一个全新的程序

