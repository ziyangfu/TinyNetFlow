基本准则
在构建Boost.Interprocess架构时，我遵循了一些基本原则，可以总结为以下几点：

Boost.Interprocess至少在 UNIX 和 Windows 系统中应具有可移植性。这意味着不仅要统一接口，还要统一行为。这就是Boost.Interprocess 选择内核或文件系统持久性作为共享内存和命名同步机制的原因。共享内存的进程持久性也是可取的，但在 UNIX 系统中很难实现。
Boost.Interprocess进程间同步原语应该等同于线程同步原语。Boost.Interprocess旨在拥有与 C++ 标准线程 API 兼容的接口。
Boost.Interprocess架构应该是模块化、可定制且高效的。这就是为什么Boost.Interprocess 基于模板和内存算法，索引类型、互斥类型和其他类都是可模板化的。
Boost.Interprocess架构应允许与基于线程的编程相同的并发性。定义了不同的互斥级别，以便一个进程可以在扩展共享内存向量时并发分配原始内存，而另一个进程可以安全地搜索命名对象。
Boost.Interprocess容器对Boost.Interprocess一无所知。所有特定行为都包含在类似 STL 的分配器中。这允许 STL 供应商稍微修改（或更好地说，概括）他们的标准容器实现并获得完全兼容 std::allocator 和 boost::interprocess::allocator 的容器。这也使Boost.Interprocess 容器与标准算法兼容。
Boost.Interprocess建立在 3 个基本类之上：内存算法，段管理器和托管内存段

机器翻译自原文：
https://www.boost.org/doc/libs/1_87_0/doc/html/interprocess/architecture.html