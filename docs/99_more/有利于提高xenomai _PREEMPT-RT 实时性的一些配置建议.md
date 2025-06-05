[![返回主页](/skins/custom/images/logo.gif)](https://www.cnblogs.com/wsg1100/)

[沐多](https://www.cnblogs.com/wsg1100)
=====================================

*   [博客园](https://www.cnblogs.com/)
*   [首页](https://www.cnblogs.com/wsg1100/)
*   [新随笔](https://i.cnblogs.com/EditPosts.aspx?opt=1)
*   [联系](https://msg.cnblogs.com/send/%E6%B2%90%E5%A4%9A)
*   [管理](https://i.cnblogs.com/)
*   [订阅](javascript:void(0)) [![订阅](/skins/coffee/images/xml.gif)](https://www.cnblogs.com/wsg1100/rss/)

随笔\- 85  文章- 0  评论- 130  阅读- 24万 

[【原创】有利于提高xenomai /PREEMPT-RT 实时性的一些配置建议](https://www.cnblogs.com/wsg1100/p/12730720.html "发布于 2020-04-19 11:22")
=================================================================================================================

![【原创】有利于提高xenomai /PREEMPT-RT 实时性的一些配置建议](https://img2024.cnblogs.com/blog/1250138/202501/1250138-20250105151703784-74998945.png) xenomai /PREEMPT-RT 实时性优化指南

版权声明：本文为本文为博主原创文章，转载请注明出处 [https://www.cnblogs.com/wsg1100。如有错误，欢迎指正。](https://www.cnblogs.com/wsg1100%E3%80%82%E5%A6%82%E6%9C%89%E9%94%99%E8%AF%AF%EF%BC%8C%E6%AC%A2%E8%BF%8E%E6%8C%87%E6%AD%A3%E3%80%82)  
@

目录

*   [一、前言](#一前言)
    *   [1\. 什么是实时](#1-什么是实时)
    *   [2\. 实时分类](#2-实时分类)
        *   [硬实时(Hard real time system)](#硬实时hard-real-time-system)
        *   [强实时(Firm real time system)](#强实时firm-real-time-system)
        *   [软实时(Soft real time system)](#软实时soft-real-time-system)
    *   [3.常见的RTOS](#3常见的rtos)
    *   [4\. latency和jitter](#4-latency和jitter)
*   [二、实时性的影响因素](#二实时性的影响因素)
    *   [1.硬件](#1硬件)
        *   [CPU架构](#cpu架构)
        *   [Cache](#cache)
            *   [X86平台优化](#x86平台优化)
            *   [ARM64平台优化](#arm64平台优化)
        *   [TLB](#tlb)
        *   [分支预测](#分支预测)
        *   [Hyper-Threading](#hyper-threading)
        *   [电源管理与调频](#电源管理与调频)
        *   [Multi-Core](#multi-core)
        *   [DMA](#dma)
        *   [other](#other)
    *   [2.BIOS（X86平台）](#2biosx86平台)
        *   [SMI(System Management Interrupts)](#smisystem-management-interrupts)
    *   [3.软件](#3软件)
    *   [4\. GPU](#4-gpu)
*   [三、优化措施](#三优化措施)
    *   [1\. BIOS\[x86\]](#1-biosx86)
        *   [BIOS配置](#bios配置)
            *   [Xeon Processors](#xeon-processors)
            *   [Atom & Core Processors](#atom--core-processors)
        *   [OS SMI配置](#os-smi配置)
            *   [禁用所有 SMI 源](#禁用所有-smi-源)
            *   [有选择地禁用 SMI 源](#有选择地禁用-smi-源)
    *   [2\. 硬件](#2-硬件)
    *   [3\. Linux](#3-linux)
        *   [3.1 Kernel CMDLINE](#31-kernel-cmdline)
            *   [cpu隔离](#cpu隔离)
            *   [Full Dynamic Tick](#full-dynamic-tick)
            *   [Offload RCU callback](#offload-rcu-callback)
            *   [中断](#中断)
            *   [禁用irqbanlance](#禁用irqbanlance)
            *   [intel 核显配置\[x86\]](#intel-核显配置x86)
            *   [禁用调频](#禁用调频)
            *   [hugepages](#hugepages)
            *   [nmi_watchdog\[x86\]](#nmi_watchdogx86)
            *   [nosoftlockup](#nosoftlockup)
            *   [CPU特性\[x86\]](#cpu特性x86)
        *   [3.2 内核构建配置](#32-内核构建配置)
*   [三、软件方面](#三软件方面)
*   [四、优化结果对比](#四优化结果对比)
    *   *   [1\. 原始性能测试。](#1-原始性能测试)
        *   [2\. 优化BIOS设置。](#2-优化bios设置)
        *   [3\. Linux配置优化。](#3-linux配置优化)
        *   [4\. 裁剪桌面。](#4-裁剪桌面)
        *   [5\. Full Dynamic Tick启用前后对比](#5-full-dynamic-tick启用前后对比)
        *   [6.桌面、rcu、tick前后比对](#6桌面rcutick前后比对)
        *   [7.总对比](#7总对比)
*   [四、实时性能测试](#四实时性能测试)
    *   [X86_64平台](#x86_64平台)
    *   [ARM64平台(瑞芯微RK3588)](#arm64平台瑞芯微rk3588)
*   [相关链接](#相关链接)

本文讲述一些有利于提高xenomai实时性的配置建议，部分针对X86架构，但它们的底层原理相通，同样适用于其他CPU架构和系统，希望对你有用。

一、前言
----

### 1\. 什么是实时

“实时”一词在许多应用领域中使用，人们它有不同的解释，并不总是正确的。人们常说，如果控制系统能够对外部事件做出快速反应，那么它就是实时运行的。根据这种解释，如果系统速度快，则系统被认为是实时的。然而，“快”具有相对含义，并未涵盖表征这些类型系统的主要属性。

我们来看一下，在自然界中，生物在栖息地中的实时行为，这些行为与它们的速度无关。例如，乌龟对来自其栖息地的外部刺激的反应，与猫对其栖息地的外部反应一样有效。虽然乌龟比猫慢很多，但就绝对速度而言，它要处理的事件与它可以协调的动作成正比，这是任何动物在环境中生存的必要条件。

相反，如果生物系统所处的环境，引入了速度超过其处理能力的事件，其行为将不再有效，动物的生存也会受到损害。比如，一只苍蝇可以被苍蝇拍捕捉到，一只老鼠可以被陷阱捕捉到，或者一只猫可以被高速行驶的汽车撞倒。在这些例子中，苍蝇拍、陷阱和汽车代表了动物的异常和异常事件，超出了它们的实时能力范围，可能严重危及它们的生存。  
![](https://wsg-blogs-pic.oss-cn-beijing.aliyuncs.com/xenomai/image-20211031100857737.png)

前面的例子表明，实时并没有人们想象的那样快，而是与系统运行的环境严格相关。

**实时系统是必须在设置的截止时间内对环境中的事件做出反应的系统，否则会产生严重的后果。**

![](https://wsg-blogs-pic.oss-cn-beijing.aliyuncs.com/xenomai/rt-dele.png)

再比如，船舶的制导系统可能看起来是一个非实时系统，因为它的速度很低，而且通常有“足够”的时间（大约几分钟）来做出控制决定。尽管如此，根据我们的定义，它实际上是一个实时系统。

### 2\. 实时分类

根据错过截止时间产生的后果，实时任务可以分为三类：

#### 硬实时(Hard real time system)

如果在截止时间之后产生结果，可能**对受控系统造成灾难性后果**，则该任务是硬实时任务。

硬任务的例子可以在安全关键系统中找到，并且通常与传感、驱动和控制活动有关，例如：

*   汽车安全气囊的检测与控制；
*   反导弹系统要求硬实时。反导弹系统由一系列硬实时任务组成。反导系统必须首先探测所有来袭导弹，正确定位反导炮，然后在导弹来袭之前将其摧毁。所有这些任务本质上都是硬实时的，如果反导弹系统有任何一个任务失败都将无法成功拦截来袭导弹。

#### 强实时(Firm real time system)

如果在截止日期之后产生结果**对系统无用，但不会造成任何损害**，则该任务是强实时任务。

在网络应用程序和多媒体系统中找到，在这些系统中，跳过一个数据包或一个视频帧比长时间延迟处理更重要。 因此，它们包括以下内容：

*   视频播放；
*   音/视频编解码中，没有在设置的码率时序范围内执行完，产生结果都是无用的丢弃即可，继续下一轮读取；
*   在线图像处理；

#### 软实时(Soft real time system)

如果实时任务在截止日期之后产生结果**仍然对系统有用，尽管会导致性能下降**，则该任务是软实时任务。

软任务通常与系统-用户交互有关，有点延迟什么的并不影响，只是体验稍差点。 因此，它们包括：

*   用户界面的命令解释器；
    
*   处理来自键盘的输入数据；
    
*   在屏幕上显示消息；
    
*   网页浏览等；
    

### 3.常见的RTOS

小型实时操作系统 UCOS、FreeRTOS、RT-Thread…

大型实时操作系统 RT linux、VxWorks、QNX、sylixOS…

![](https://wsg-blogs-pic.oss-cn-beijing.aliyuncs.com/xenomai/image-20220516193335375.png)

### 4\. latency和jitter

硬实时系统是必须在设置的截止时间内对环境中的事件做出反应的系统。硬实时操作系统应具备的最重要特性之一是**确定性、可预期性。**

操作系统的实时性能通常用latency或jitter来表示。**事件预期发生与实际发生的时间之间的时间称为延迟(latency)，实际发生的最大时间与最小时间之间的差值称为抖动(Jitter)，两者均可表示实时性**。 根据实时性的定义，延迟必须是确定的，不能超过deadline，否则将会产生严重的后果。

![](https://wsg-blogs-pic.oss-cn-beijing.aliyuncs.com/xenomai/jitter-latency1.png)

当我们针对实时应用场景评估硬件和实时系统时，通常可以简化为对**实时性能**和**硬件资源**的考量，即对于一个应用场景，实时性满足的情况下，硬件性能也满足。

在否决定使用一个实时系统时，需要结合具体应用场景来评估该实时系统是否符合，若不符合则需要考虑对现有系统优化或者更换方案。

二、实时性的影响因素
----------

硬实时操作系统应具备的最重要特性之一是**确定性、可预测性**，系统应该保证满足所有关键时序约束。然而，这取决于一系列因素，这些因素涉及硬件的架构特征、内核中采用的机制和策略，以及用于实现应用程序的编程语言、软件设计等。

**根据实时性定义，计算机系统中所有导致程序执行时间不确定的因素，都是实时性影响因素。**

### 1.硬件

#### CPU架构

硬件方面，第一个影响调度可预测性的是处理器本身。处理器的内部特性是不确定性的第一个原因，例如指令预取、流水线操作、分支预测、高速缓存存储器和直接存储器访问（DMA）机制。这些特性虽然改善了处理器的平均性能，但它们引入了非确定性因素，这些因素阻止了对最坏情况执行时间WCET（Worst-caseExecutionTime）的精确估计。

> 高端CPU，如I5、I7实时性不一定有低端的赛扬、atom系列的好，芯片的设计本身定位就是高吞吐量而不是实时性，**实时性与吞吐量不可兼得**。

#### Cache

*   CPU 里的 L1 Cache 或者 L2 Cache，访问延时是内存的 1/15 乃至 1/100，想要追求极限性能，需要尽可能地多从 CPU Cache 里面拿数据，减少cache miss，上面的分配CPU专门对实时任务服务就是对非共享的L1 、L2 Cache的充分优化。
    
*   对于L3 Cache，多个cpu核与GPU共享，无法避免非实时任务及GUI争抢L3 Cache对实时任务的影响。
    

##### X86平台优化

为此intel 推出了资源调配技术(Intel RDT)，提供了两种能力：监控和分配。Intel RDT提供了一系列分配(资源控制)能力，包括缓存分配技术(Cache Allocation Technology, CAT)，代码和数据优先级(Code and Data Prioritization, CDP) 以及 内存带宽分配(Memory Bandwidth Allocation, MBA)。该技术旨在通过一系列的CPU指令从而允许用户直接对每个CPU核心（附加了HT技术后为每个逻辑核心）的L2缓存、L3缓存（LLC--Last Level Cache ）以及内存带宽进行监控和分配。

> RDT一开始是为解决云计算的问题，在云计算领域虚拟化环境中，宿主机的资源（包括CPU cache和内存带宽）都是共享的。这带来一个问题就是：如果有一个过度消耗cache的应用耗尽了L3缓存或者大量的内存带宽，将无法保障其他虚拟机应用的性能。这种问题称为 noisy neighbor。
> 
> 同样对于我们的实时系统也是类似：由于L3 Cache多核共享，如果有一个过度消耗cache的**非实时应用**耗尽了L3缓存或者大量的内存带宽，将无法保障xenomai实时应用的性能。
> 
> 以往虚拟化环境中解决方法是通过控制虚拟机逻辑资源(cgroup)但是调整粒度太粗，并且无法控制处理器缓存这样敏感而且稀缺的资源。为此Intel推出了RDT技术。在Intel中文网站的 通过[英特尔® 资源调配技术优化资源利用](https://www.intel.cn/content/www/cn/zh/architecture-and-technology/optimize-resource-utilization-rdt-animation.html)视频形象介绍了RDT的作用。
> 
> Intel的Fenghua Yu在Linux Foundation上的演讲 [Resource Allocation in Intel® Resource Director Technology](https://01.org/intel-rdt-linux/blogs/fyu1/2017/resource-allocation-intel%C2%AE-resource-director-technology) 可以帮助我们快速了解这项技术。
> 
> 总的来说，RDT让我们实现了控制处理器缓存这样敏感而且稀缺的资源，对我们对实时性能提升有很大帮助(不仅限于xenomai，RTAI、PREEMPT-RT均适用)。

*   [CAT](https://www.intel.com/content/dam/www/public/us/en/documents/white-papers/cache-allocation-technology-white-paper.pdf)（缓存分配技术，Cache Alocation Technology），对最后一级缓存（L3 Cache）实现分区，用户可以通过限制每个核心能够向其中分配缓存行的LLC数量，将LLC的部分分配给特定核心，使用该技术可以提升实时任务Cahe命中率，减少MSI延迟和抖动，进而提升实时性能。（不是所有intel处理器具有该功能，一开始只有服务器CPU提供该支持,据笔者了解，6代以后的CPU基本支持CAT。关于CAT 见[github](https://github.com/intel/intel-cmt-cat))，对于大多数Linux发行版，可直接安装使用该工具，具体的cache分配策略可根据后面的资源隔离情况进行。

    sudo apt-get install  intel-cmt-cat
    

##### ARM64平台优化

这里以瑞芯微RK3568平台为例，其LLC为多核共享，可以划分(其他RK3588/3562等没有L3)，让实时任务独占LLC，提高实时性，RK3568划分LLC独占方式如下：

a) cache 分片  
ARM Cortex-A55 架构上面支持对L3空间进行划分，原理为：Cortex-A55 L3 mem空间划分为4块，可以配  
置每个CPU使用4块L3中的哪几块，在rkbin中的RKBOOT/RK3568MINIALL.ini文件进行配置：

    [BOOT1_PARAM]
    
    WORD_3=0xc333


`WORD_3`的值0xc333表示（以P0、P1、P2、P3表示L3的4块空间）：  
cpu0、cpu1 共享L3的P0、P1。  
cpu2、cpu3 共享L3的P2、P3。

WORD_3配置值，详细说明如下 ：  
bit0~bit3：分配给cpu0的4份L3的mask bit， bit0为1，表示L3的第一份分给cpu0，bit1为1，表示L3的  
第二份分给cpu0，以此类推。  
bit4~bit7：分配给cpu1的4份L3的mask bit。  
bit8~bit11：分配给cpu2的4份L3的mask bit。  
bit12~bit15：分配给cpu3的4份L3的mask bit。  
配置后可以通过下面开机LOG确认


    INFO: L3 cache partition cfg-c333


此为分配cpu3 256k L3。

b）隔离核心  
bootargs添加 `isolcpus=3 nohz_full=3` ，将核心cpu3隔离出来，不参与系统任务调度，并作为实时核心，关于内核参数详见后文。

#### TLB

与cache性质一致。

#### 分支预测

现代 CPU 的流水线级数非常长，一般都在10级以上，指令分支判断错误（Branch Mispredict）的时间代价昂贵。如果判断预测正确，可能只需要一个时钟周期；如果判断错误，就还是需要10-20 左右个时钟周期来重新提取指令。

如下为对同一随机组数，排序与未排序情况下for循环测试：

![](https://wsg-blogs-pic.oss-cn-beijing.aliyuncs.com/xenomai/image-20220515222133653.png)

数据有规律和无规律两种情况下同一段代码执行时间相差巨大。

现代 CPU 的分支预测正确率已经可以在一般情况下维持在 95% 以上，所以当分支存在可预测的规律的时候，还是以性能测试的结果为最终的优化依据。

#### Hyper-Threading

人们对CPU的性能的追求是无止境的，在CPU性能不断优化提高过程中，对于单一流水线，最佳情况下，IPC 也只能到 1。无论做了哪些流水线层面的优化，即使做到了指令执行层面的乱序执行，CPU 仍然只能在一个时钟周期里面取一条指令。

为使IPC>1，诞生了多发射（Mulitple Issue）和超标量（Superscalar）技术，伴随的是每个CPU流水线上各种运算单元的增加。但是当处理器在运行一个线程，执行指令代码时，一方面很多时候处理器并不会使用到全部的计算能力，另一方面由于CPU在代码层面运行前后依赖关系的指令，会遇到各种冒险问题，这样CPU部分计算能力就会处于空闲状态。

为了进一步“压榨”处理器，那就找没有依赖关系的指令来运行好，即另一个程序。一个核可以分成几个逻辑核，来执行多个控制流程，这样可以进一步提高并行程度，这一技术就叫超线程，又称同时多线程（Simultaneous Multi-Threading，简称 SMT）。

由于超线程技术通过双份的 PC 寄存器、指令寄存器、条件码寄存器，在逻辑层面伪装为2个CPU，但指令译码器和ALU是公用的，这就造成实时任务运行时在CPU执行层面的不确定性，造成非实时线程与实时线程在同一物理核上对CPU执行单元的竞争，影响实时任务实时性。

#### 电源管理与调频

我们知道CPU场效应晶体管FET构成，其简单示意图如下。

![](https://wsg-blogs-pic.oss-cn-beijing.aliyuncs.com/xenomai/image-20220516193803636.png)

当输入高低电平时，CL被充放电，假设充放电a焦耳的能量。因为CL很小，这个a也十分的小，几乎可以忽略不计。为了提高CPU性能，不断提高处理器的时钟频率，但如果我们以1GHz频率翻转这个FET，则能量消耗就是a × 10^9，这就不能忽略了，再加上CPU中有几十亿个FET，消耗的能量变得相当可观。

> 详细的参考：[https://zhuanlan.zhihu.com/p/56864499](https://zhuanlan.zhihu.com/p/56864499)

为了省电，让操作系统随着工作量不同，动态调节CPU频率和电压。但是调频会导致CPU停顿（CPU停顿时间10us~500us不等），运行速度降低导致延迟增加，严重影响实时性能。

除了调频以外，另一个严重影响实时性的是，系统进入更深层次的省电睡眠状态，这时的唤醒延迟长达几十毫秒。

#### Multi-Core

接收 IRQ 的 CPU 可能不是响应者休眠的 CPU，在这种情况下，前者必须向后者发送重新调度请求，以便它恢复响应者。这通常是通过处理器间中断完成的，也就是IPI，IPI的发送和处理进一步增加了延迟。

中断周期及测试时长

最小

平均

最大

100us 21h

0.086us

0.184us

4.288us

此外，多核总线共享、LLC共享，NUMA架构远端内存访问等，均会导致访问延迟不确定。

#### DMA

DMA突发传输不确定占用总线带宽，影响程序执行；  
总线动态调频省电等

#### other

*   DMA传输不确定性
    
*   其他影响因素有**内存、散热**。
    

提升内存频率可降低内存访问延时；使用双通道内存，这两个内存CPU可分别寻址、读取数据，从而使内存的带宽增加一倍，数据存取速度也相应增加一倍（理论上），内存访问延时得到缩短，进而提升系统的实时性能；

处理器散热设计不好，温度过高时会引发CPU降频保护，系统运行频率降低影响实时性，热设计应确保在高工作量时的温度不会引发降频。

对于X86 CPU，双通道内存性能是单通道内存的2. 5倍以上；正确的热设计可使实时性提升1.4倍以上。

### 2.BIOS（X86平台）

BIOS堪称是整个系统中的扫地僧，它运行于系统管理模式，用于实现电源管理和硬件控制功能，在系统启动时配置硬件工作环境，系统启动后之后便悄然退居幕后，持续监测系统的稳定运行状态。比如，处理系统事件，如内存或芯片组错误，一旦遇到如CPU温度异常等重大故障或潜在危险时，BIOS会迅速介入并采取紧急措施，例如实施强制降频或关机等操作以保障系统安全。  
因此，对于实时操作系统来说，对BIOS进行精细化配置显得尤为重要。经过优化的BIOS设置与仅仅依赖默认设置相比，在实时性能表现上可能带来显著提升，两者之间的差距甚至可高达9倍之多。

#### SMI(System Management Interrupts)

系统管理中断(System Management Interrupts)或 SMI 是最高优先级的特殊中断，导致 x86 CPU 进入系统管理模式SMM(System Management Mode)，这是实模式的一种变体，用于执行 BIOS 实现的某些处理程序，在这种模式下，包括操作系统在内的所有正常执行都暂停。SMI 不通过中断控制器，它们在指令之间被 CPU 逻辑检测到，并从那里无条件地调度。这给实时系统带来了关键问题：

*   SMI 可以在任何时候在不确定的时间内抢占实时代码，并且不能被内核软件屏蔽或抢占。实际上，内核软件甚至不知道正在进行的 SMI 请求。
*   转换到 SMM 上下文或从 SMM 上下文转换需要 CPU 保存/恢复其大部分寄存器文件，切换到不同的 CPU 模式。对于多核系统，BIOS 甚至可以等待所有 CPU 内核进入 SMM，然后再序列化待处理的 SMI 请求的执行。这是意外延迟的另一个来源。
*   SMI 调用的 SMM 处理程序是在 BIOS 中实现的，因此它们的实现对我们来说是不透明的。我们可能只是观察到其中一些引起延迟变大的点（例如，很常见看到USB相关SMI的300微秒延迟）。

现今，几乎所有的x86架构台式机及服务器级别的硬件在很大程度上都依赖于系统管理中断（SMI）来执行各种内部管理操作以及所谓的优化功能。然而，在面对嵌入式应用和实时约束条件时，情况有所不同。  
针对支持嵌入式应用设计的片上系统（SoC）平台通常在这方面表现更优，更能满足实时性需求。因此，在购买此类硬件之前，强烈建议您提前向供应商详细了解其关于SMI问题的解决方案，或者自行测试硬件在处理SMI方面的性能表现，以免影响实际应用中实时性的要求。不是淘宝随便买一台X86板子就能保证系统实时性的。

### 3.软件

*   操作系统：
    *   调度算法，同步机制，优先级倒置，信号量类型，通信语义和中断处理等。
    *   异常处理方式：中断线程化、中断上下部
    *   内存管理
    *   内存访问时间不确定性：MMU、cache、NUMA
    *   惰性分配策略：Pagefault、CoW(Copy of Write)
    *   内存分配算法时间不确定性：快速分配路径 vs 慢速分配路径（内存合并和回收）、OOM (Out Of Memory) killer、swap
    *   RT调度策略：FIFO、RR...
    *   实时驱动
*   资源的分配隔离：分配CPU专门对实时任务服务、将多余中断隔离到非实时任务CPU上，分配CPU专门对实时任务服务可使L1 、L2 Cache只为实时任务服务。
*   编程语言（缺乏对时间的明确支持）：
    *   编译型语言
    *   汇编、C、C++、Rust、Golang等，编程语言GC管理
    *   完全动态链接、运行时链接、运行时重定位
    *   语言特性
    *   解释型语言和即时编译型语言，完全与实时不沾边
*   实时任务的设计，良好的软件设计能更好的发挥实时性能。
    *   优先级如何设置
    *   任务间交互设计
    *   软件可维护性、时序约束可分析与可验证性
    *   过载和容错处理机制
    *   应用算法复杂度（计算时间确定性）
*   其他，虚拟化、GUI等 。

### 4\. GPU

硬件上GPU与CPU共享L3 Cache ，因此GUI会影响实时任务的实时性。intel建议根据GUI任务的工作负载来固定GPU的运行频率，且频率尽可能低。减小GPU对实时任务实时性的影响。

三、优化措施
------

原则：**降低不确定性，提高可预期性，在此基础上，再提高速度，降低延时。**

比如，我们需要在确定的时间内从广州到深圳，如果驾车，途中会遇到多少个红绿灯，有无堵车  
等等，有很多不确定性。但是如果我们换坐动车，就比驾车更具确定性，在此基础上我们提高速度，换坐高铁，广州到深圳的延时将变得更小。

### 1\. BIOS\[x86\]

#### BIOS配置

Disable Features

Intela Hyper-Threading Technology.

Intel SpeedStep.

Intel Speed Shift Technology

C-States: Gfx RC6.

GT PM Support.

PCH Cross Throttling.

PCI Express_ Clock Gating.

Delay Enable DMI ASPM,DMI Link ASPM Control.

PCle _ASPM and SATA Aggressive LPM Support.

（For Skylake and Kaby Lake, also consider disabling Gfx Low Power Mode and USB Periodic SMl in BIOS.）

Enable Features

Legacy lO Low Latency

Gfx Frequency

Set to fixed value as low as possible according to proper workload

Memory Frequency

SA GV Fixed High

上面信息有些过时，但X86平台有新有旧，还作保留，新处理器平台以下面的为准，来源[intel ECI](https://eci.intel.com)

> 更新于2025.4

##### Xeon Processors

Setting Name

Option

Setting Menu

Logical Processor

Disabled

Processor Settings

Uncore Frequency RAPL

Disabled

Processor Settings

Local Machine Check Exception

Disabled

Processor Settings

System Profile

Custom

System Profile Settings

Optimized Power Mode

Disabled

System Profile Settings

Turbo Boost

Disabled

System Profile Settings

C1E

Disabled

System Profile Settings

C-States

Disabled

System Profile Settings

Uncore Frequency

Maximum

System Profile Settings

Dynamic Load Line Switch

Disabled

System Profile Settings

Energy Efficient Policy

Performance

System Profile Settings

CPU Interconnect Bus Link Power Management

Disabled

System Profile Settings

PCI ASPM L1 Link Power Management

Disabled

System Profile Settings

Workload Configuration

IO Sensitive

System Profile Settings

##### Atom & Core Processors

Setting Name

Option

Setting Menu

Hyper-Threading

Disabled

Intel Advanced Menu ⟶ CPU Configuration

Intel (VMX) Virtualization

Disabled

Intel Advanced Menu ⟶ CPU Configuration

Intel(R) SpeedStep

Disabled

Intel Advanced Menu ⟶ Power & Performance ⟶ CPU - Power Management Control

Turbo Mode

Disabled

Intel Advanced Menu ⟶ Power & Performance ⟶ CPU - Power Management Control

C States

Disabled

Intel Advanced Menu ⟶ Power & Performance ⟶ CPU - Power Management Control

RC6 (Render Standby)

Disabled

Intel Advanced Menu ⟶ Power & Performance ⟶ GT - Power Management Control

Maximum GT freq

Lowest (usually 100MHz)

Intel Advanced Menu ⟶ Power & Performance ⟶ GT - Power Management Control

SA GV

Fixed High

Intel Advanced Menu ⟶ Memory Configuration

VT-d

Enabled* (see footnote)

Intel Advanced Menu ⟶ System Agent (SA) Configuration

PCI Express Clock Gating

Disabled

Intel Advanced Menu ⟶ System Agent (SA) Configuration ⟶ PCI Express Configuration

Gfx Low Power Mode

Disabled

Intel Advanced Menu ⟶ System Agent (SA) Configuration ⟶ Graphics Configuration

ACPI S3 Support

Disabled

Intel Advanced Menu ⟶ ACPI Settings

Native ASPM

Disabled

Intel Advanced Menu ⟶ ACPI Settings

Legacy IO Low Latency

Enabled

Intel Advanced Menu ⟶ PCH-IO Configuration

PCH Cross Throttling

Disabled

Intel Advanced Menu ⟶ PCH-IO Configuration

Delay Enable DMI ASPM

Disabled

Intel Advanced Menu ⟶ PCH-IO Configuration ⟶ PCI Express Configuration

DMI Link ASPM

Disabled

Intel Advanced Menu ⟶ PCH-IO Configuration ⟶ PCI Express Configuration

Aggressive LPM Support

Disabled

Intel Advanced Menu ⟶ PCH-IO Configuration ⟶ SATA And RST Configuration

USB Periodic SMI

Disabled

Intel Advanced Menu ⟶ LEGACY USB Configuration

#### OS SMI配置

默认情况下，Xenomai核心中存在的SMI（系统管理中断）问题解决代码尝试通过识别底层芯片组来检测可能出现的与SMI/SMM相关的潜在问题。这对应于以下内核参数设置：

    xenomai.smi=detect


这样启动Xenomai后，xenomai通过向内核日志发出以下警告来警告您由于当前芯片组启用 SMI 而导致的潜在问题：

    Xenomai: SMI-enabled chipset found, but SMI workaround disabled


此时，你需要在负载下运行 Xenomai 的 latency 测试程序，重点关注是否存在病态延迟（这里的“病态”指的是超过某个阈值，比如100微秒以上的延迟），这样来确定系统的实时性能是否受到SMI等因素的影响。  
如果没有观察到任何此类延迟点，则此警告是无害的，可以按以下步骤操作：

*   通过完全禁用 Xenomai 的 SMI 检测来关闭此警告。将以下参数添加到内核命令行：

    xenomai.smi=disabled
    
*   跳过此小节的其余内容

否则，如果确实观察到任何病态延迟，那么 SMI有问题，Xenomai 提供了两种解决方法，这可能对有所帮助。

##### 禁用所有 SMI 源

禁用所有 SMI 源，如下所示：

    xenomai.smi=enabled


禁用SMI后可能导致系统工作异常，所以必须检查

*   内核日志消息不能包含以下消息：

    demsg
    ...
    Xenomai: SMI workaround failed!
    ...
    

> 若存在上面的内核信息，说明该操作无效，需要尝试另一种方法‘有选择地禁用 SMI 源’。

*   禁用所有SMI后系统上的每个设备（例如键盘、鼠标、NIC）仍能正常响应。
*   即使在持续的高负载条件下，主板也不会过热。如果在对系统进行压力测试时出现意外和临时重启，那么应该立即重新启用 SMI，并放弃这种方法。
*   长时间测试也不再显示任何病态延迟。

如果只有特定设备无法正常工作，则可能需要 SMI，在这种情况下，无法全局禁用它们。  
在系统过热的情况下也是如此：您可以尝试使用于热控制的 SMI 源保持启用状态，从而禁用其他源。

##### 有选择地禁用 SMI 源

为了有选择地控制 SMI 源，请查看英特尔芯片组的文档，查找有关SMI_EN寄存器的讨论，以及与为此类芯片组定义的 SMI 源相对应的位值，你可以将位掩码传递给下面的内核参数，以便 Xenomai 将尝试禁用值中清除其位的每个 SMI 源，而启用其他源：

    xenomai.smi_mask=<enable-mask>


再次，检查内核日志消息是否不包含:

    Xenomai: SMI workaround failed!


如果包含此消息，则无法使用 Xenomai SMI解决方法来避免SMI，则应检查 BIOS 中是否有可能导致 SMI 的设置。  
使用谨慎的增量方法，优化禁用的源集，您应该尝试仅停止导致病态延迟的 SMI 源，保持系统的其余部分安全和理智。每次迭代都应通过运行标准 Xenomai 延迟测试来重新验证当前状态。

### 2\. 硬件

除处理器外，内存方面，使用双通道内存，尽可能高的内存频率。

散热当面，针对处理器工作负载设计良好的散热结构, 否则芯片保护会强制降频，频率调整CPU会停顿几十上百us。

### 3\. Linux

xenomai基于linux，xenomai作为一个小的实时核与linux共存，xenomai并未提供完整的硬件管理机制，许多硬件配置是linux 驱动掌管的，必须让linux配置好，给xenomai提供一个好的硬件环境，让xenomai充分发挥其RTOS的优势，**主要宗旨：尽可能的不让linux非实时部分影响xenomai，无论是软件还是硬件**。

#### 3.1 Kernel CMDLINE

注：以下配置信息，明确提到‘xenomai’的为xenomai特定配置，其余为xenomai和preempt-RT的通用配置。

##### cpu隔离

多核情况下，设置内核参数`isolcpus=[cpu列表]`,将列表中的CPU从linux内核SMP平衡和调度算法中剔除，将剔除的CPU用于RT应用。如4核CPU平台将第3、4核隔离来做RT应用。

> CPU编号从"0"开始，列表的表示方法有三种：  
> numA,numB,...,numN  
> numA-numN  
> 以及上述两种表示方法的组合：  
> numA,...,numM-numN  
> 例如：isolcpus=0,3,4-7表示隔离CPU0、3、4、5、6、7.

    GRUB_CMDLINE_LINUX="isolcpus=2,3"


以上只是linux不会调度普通任务到CPU2和3上运行,**这是基础**，此时还需要设置xenomai方面的CPU隔离，方法一，任务通过函数`pthread_attr_setaffinity_np()`设置xenomai任务只在CPU3和4上调度，隔离后的CPU的L1、L2缓存命中率相应的也会得到提高。

    cpu_set_t cpus;
    CPU_ZERO(&cpus);
    CPU_SET(2, &cpus);//将线程限制在指定的cpu2上运行
    CPU_SET(3, &cpus);//将线程限制在指定的cpu3上运行
    ret = pthread_attr_setaffinity_np(&tattr, sizeof(cpus), &cpus);


方法二，向xenomai设置内核参数`supported_cpus`,指定xenomai支持的CPU，xenomai任务会自动放到cpu2、cpu3上运行，注意对于xenomai3.2以上版本，supported_cpus必须包含CPU0，否则xenomai内核无法启动！！。

> xenomai 内核参数`supported_cpus`与linux不同，`supported_cpus`是一个16进制数，每bit置位表示支持该CPU，要支持CPU2、CPU3，需要置置位bit2、bit3，即`supported_cpus=0x0c`(00001100b）。

    GRUB_CMDLINE_LINUX="audit=0 isolcpus=2,3 xenomai.supported_cpus=0x0C"


注：linux内核参数`isolcpus=CPU编号列表`是基础，否则若不隔离linux任务，后面的xenomai设置将没任何意义。

##### Full Dynamic Tick

将CPU2、CPU3作为xenomai使用后，由于xenomai调度是完全基于优先级的调度器，并且我们已将linux任务从这两个cpu上剔除，CPU上Tick也就没啥用了，避免多余的Tick中断影响实时任务的运行，需要将这两个cpu配置为Full Dynamic Tick模式，即关闭tick。通过添加linux内核参数`nohz_full=[cpu列表]`配置。

`nohz_full=[cpu列表]`在使用`CONFIG_NO_HZ_FULL = y`构建的内核中才生效。

    GRUB_CMDLINE_LINUX="audit=0  isolcpus=2,3 xenomai.supported_cpus=0x0c nohz_full=2,3"


为什么是linux内核参数呢？双核下时间子系统中分析过，每个CPU的时钟工作方式是linux初始化并配置工作模式的，xenomai最后只是接管而已，所以这里是通过linux内核参数配置。

**注意：boot CPU(通常是0号CPU)会无条件的从列表中剔除。这是一个坑~**

    start_kerel()
    ->tick_init()
    	->tick_nohz_init()
    void __init tick_nohz_init(void)
    {
    .......
    cpu = smp_processor_id();
    
    	if (cpumask_test_cpu(cpu, tick_nohz_full_mask)) {
    		pr_warn("NO_HZ: Clearing %d from nohz_full range for timekeeping\n",
    			cpu);
    		cpumask_clear_cpu(cpu, tick_nohz_full_mask);
    	}
    ......
    }


##### Offload RCU callback

从引导选择的CPU上卸载RCU回调处理,使用内核线程 “rcuox / N”代替，通过linux内核参数`rcu_nocbs=[cpu列表]`指定的CPU列表设置。这对于HPC和实时工作负载很有用，这样可以减少卸载RCU的CPU上操作系统抖动。

> "rcuox / N",N表示CPU编号，‘x’：'b'是RCU-bh的b,'p'是RCU-preempt，‘s’是RCU-sched。

`rcu_nocbs=[cpu列表]`在使用`CONFIG_RCU_NOCB_CPU=y`构建的内核中才生效。除此之外需要设置RCU内核线程`rcuc/n`和`rcub/n`线程的SCHED_FIFO优先级值**RCU\_KTHREAD\_PRIO**，RCU\_KTHREAD\_PRIO设置为高于最低优先级线程的优先级，也就是说至少要使该优先级低于xenomai实时应用的优先级，避免xenomai实时应用迁移到linux后，由于优先级低于RCU\_KTHREAD的优先级而实时性受到影响，如下配置RCU\_KTHREAD_PRIO=0。

    General setup  --->
        RCU Subsystem  --->
            (0) Real-time priority to use for RCU worker threads
            [*] Offload RCU callback processing from boot-selected CPUs 
                (X) No build_forced no-CBs CPUs
                ( ) CPU 0 is a build_forced no-CBs CPU
                ( ) All CPUs are build_forced no-CBs CPUs


    GRUB_CMDLINE_LINUX="audit=0  isolcpus=2,3 xenomai.supported_cpus=0x06 nohz_full=2,3 rcu_nocbs=2,3 rcu_nocb_poll=1024 rcupdate.rcu_cpu_stall_suppress=1"


`rcu_nocb_poll=1024`减轻每个 CPU 唤醒其 RCU 卸载线程的责任。  
`rcupdate.rcu_cpu_stall_suppress=1`禁止显示 RCU CPU 停顿警告消息。

##### 中断

*   **中断隔离**

xenomai用户态实时应用运行时，中断优先级最高，CPU必须响应中断，虽然有ipipe会简单将非实时设备中断挂起，但是频繁的非实时设备中断产生可能引入无限延迟，也会影响实时任务的运行。

因此多，核情况下，通过内核参数`irqaffinity==[cpu列表]`，设置linux设备中断的亲和性，设置后，默认由这些cpu核来处理中断。避免了非实时linux中断影响cpu2、cpu3上的实时应用，将linux中断指定到cpu0、cpu1处理，添加参数：

    GRUB_CMDLINE_LINUX="audit=0  isolcpus=2,3 xenomai.supported_cpus=0x0c nohz_full=2,3 rcu_nocbs=2,3 irqaffinity=0,1"


以上只是设置linux中断的affinity，只能确保运行实时任务的CPU2、cpu3不会收到linux非实时设备的中断请求，保证实时性。

要指定cpu来处理xenomai实时设备中断，需要在实时驱动代码中通过函数`xnintr_affinity()`设置,绑定实时驱动中断由CPU2、CPU3处理代码如下。

        cpumask_t irq_affinity;
        ...
        cpumask_clear(&irq_affinity);
        cpumask_set_cpu(2, &irq_affinity);
        cpumask_set_cpu(3, &irq_affinity);
        ...
        if (!cpumask_empty(&irq_affinity)){
        	xnintr_affinity(&pIp->irq_handle,irq_affinity);	/*设置实时设备中断的affinity*/
        }


虽然ipipe会保证xenomai 实时中断在任何CPU都会优先处理，在实时设备中断比较少的场合，我觉得把linux中断与实时中断分开比较好；如果实时设备中断数量较多，如果隔离就会造成实时中断间相互影响中断处理的实时性，这时候不指定实时中断处理CPU比较好。

*   编写xenomai实时设备驱动程序时，中断处理程序需要尽可能的短。

##### 禁用irqbanlance

linux irqbalance 用于优化中断分配，它会自动收集系统数据以分析使用模式，并依据系统负载状况将工作状态置于 Performance mode 或 Power-save mode。简单来说irqbalance 会将硬件中断分配到各个CPU核心上处理。

*   处于 Performance mode 时，irqbalance 会将中断尽可能均匀地分发给各个 CPU core，以充分利用 CPU 多核，提升性能。
*   处于 Power-save mode 时，irqbalance 会将中断集中分配给第一个 CPU，以保证其它空闲 CPU 的睡眠时间，降低能耗。

禁用irqbanlance，避免不相干中断发生在RT任务核。发行版不同，配置方式不同，以Ubuntu为例，停止/关闭开机启动如下。

    systemctl stop irqbalance.service
    systemctl disable irqbalance.service


必要的话直接卸载irqbalance。

    apt-get remove irqbalance


x86平台还可添加参数`acpi_irq_nobalance`禁用ACPI irqbalance.

    GRUB_CMDLINE_LINUX="isolcpus=2,3 xenomai.supported_cpus=0x0c nohz_full=2,3 rcu_nocbs=2,3 irqaffinity=0,1 acpi_irq_nobalance noirqbalance"


##### intel 核显配置\[x86\]

主要针对intel CPU的核显，配置intel核显驱动模块i915，防止集成显卡更改电源状态，内核参数如下。

    GRUB_CMDLINE_LINUX="i915.enable_rc6=0 i915.enable_dc=0 i915.disable_power_well=0  i915.enable_execlists=0 i915.powersave=0"


##### 禁用调频

`cpufreq.off=1`禁用调频，调频影响实时性详见后文。

##### hugepages

使用大页有利于降低缺页异常和提高tlb命中率、以减少内存分配的频率，这里设置一次分配 1GB 的 RAM 块。

    GRUB_CMDLINE_LINUX="hugepages=1024 "


防止基于任务处理器相关性移动缓存

    GRUB_CMDLINE_LINUX="numa_balancing=disable"


##### nmi_watchdog\[x86\]

NMI watchdog是Linux的开发者为了debugging而添加的特性，但也能用来检测和恢复Linux kernel [hang](http://en.wikipedia.org/wiki/Hang_(computing))，现代多核x86体系都能支持NMI watchdog。

NMI（Non Maskable Interrupt）即不可屏蔽中断，之所以要使用NMI，是因为NMI watchdog的监视目标是整个内核，而内核可能发生在关中断同时陷入死循环的错误，此时只有NMI能拯救它。

Linux中有两种NMI watchdog，分别是I/O APIC watchdog（nmi\_watchdog=1）和Local APIC watchdog（nmi\_watchdog=2）。它们的触发机制不同，但触发NMI之后的操作是几乎一样的。一旦开启了I/O APIC watchdog（nmi_watchdog=1），那么每个CPU对应的Local APIC的LINT0线都关联到NMI，这样每个CPU将周期性地接到NMI，接到中断的CPU立即处理NMI，用来悄悄监视系统的运行。如果系统正常，它啥事都不做，仅仅是更改 一些时间计数；如果系统不正常（默认5秒没有任何普通外部中断），那它就闲不住了，会立马跳出来，且中止之前程序的运行。该出手时就出手。

避免周期中断的NMI watchdog影响xenomai实时性需要关闭NMI watchdog，传递内核参数`nmi_watchdog=0`.  
禁用超线程，传递内核参数`noht`.

    GRUB_CMDLINE_LINUX="audit=0  isolcpus=2,3 xenomai.supported_cpus=0x0c xenomai.smi_mask=1 xenomai.smi=detect nohz_full=2,3 rcu_nocbs=2,3 irqaffinity=0,1 acpi_irq_nobalance noirqbalance i915.enable_rc6=0 i915.enable_dc=0 i915.disable_power_well=0  i915.enable_execlists=0 i915.powersave=0 nmi_watchdog=0 noht"


##### nosoftlockup

linux内核参数，禁用 soft-lockup检测器在进程在 CPU 上执行的时间超过 softlockup 阈值（默认 120 秒）时禁用回溯跟踪日志记录。。

    GRUB_CMDLINE_LINUX="audit=0  isolcpus=2,3 xenomai.supported_cpus=0x0c nohz_full=2,3 rcu_nocbs=2,3 irqaffinity=0,1 acpi_irq_nobalance noirqbalance i915.enable_rc6=0 i915.enable_dc=0 i915.disable_power_well=0  i915.enable_execlists=0 i915.powersave=0 nmi_watchdog=0 nosoftlockup"


##### CPU特性\[x86\]

intel处理器相关内核参数：

*   `nosmap`
*   `nohalt`。告诉内核在空闲时,不要使用省电功能PAL\_HALT\_LIGHT。 这增加了功耗。但它减少了中断唤醒延迟，这可以提高某些环境下的性能，例如联网服务器或实时系统。
*   `mce=ignore_ce`,忽略[machine check](https://www.kernel.org/doc/Documentation/x86/x86_64/machinecheck)errors (MCE).
*   `idle=poll`,不要使用HLT在空闲循环中进行节电，而是轮询以重新安排事件。 这将使CPU消耗更多的功率，但对于在多处理器基准测试中获得稍微更好的性能可能很有用。 它还使使用性能计数器的某些性能分析更加准确。
*   `clocksource=tsc tsc=reliable`,指定tsc作为系统clocksource.
*   `intel_idle.max_cstate=0` 禁用intel\_idle并回退到acpi\_idle.
*   `processor.max_cstate=0 intel.max_cstate=0 processor_idle.max_cstate=0` 限制睡眠状态c-state。
*   `hpet=disable clocksource=tsc tsc=reliable` 系统clocksource相关配置。

    GRUB_CMDLINE_LINUX="audit=0 xenomai.allowed_group=1234 isolcpus=2,3 xenomai.supported_cpus=0x0D xenomai.smi_mask=1 xenomai.smi=detect irqaffinity=0,1 nohz_full=2,3 rcu_nocbs=2,3 rcu_nocb_poll rcupdate.rcu_cpu_stall_suppress=1 acpi_irq_nobalance numa_balancing=disable cpufreq.off=1 nosmap nosmt noirqbalance hugepages=1024  i915.enable_rc6=0 i915.enable_dc=0 i915.disable_power_well=0  i915.enable_execlists=0 processor.max_cstate=0 intel.max_cstate=0 processor_idle.max_cstate=0 intel_idle.max_cstate=0 clocksource=tsc pcie_asmp=off tsc=reliable nmi_watchdog=0 noht nosoftlockup intel_pstate=disable idle=poll nohalt mce=ignore_ce hpet=disable"
    

#### 3.2 内核构建配置

系统构建时，除以上提到的配置外（CONFIG\_NO\_HZ\_FULL = y、CONFIG\_RCU\_NOCB\_CPU=y、RCU\_KTHREAD\_PRIO=0），其他实时性相关配置如下：

CONFIG\_MIGRATION=n、CONFIG\_MCORE2=y\[x86\]、CONFIG\_PREEMPT=y、ACPI\_PROCESSOR =n\[x86\]、CONFIG\_CPU\_FREQ =n、CONFIG\_CPU\_IDLE =n；

经过以上配置后可以使用latency测试，观察配置前后的变化。关于latency，需要注意的是，测试timer-IRQ的latency时，即用`latency -t2`命令来测试时，xenomai默认使用cpu0的timer，上面提到boot CPU(通常是0号CPU)会无条件的从`nohz_full=[cpu列表]`列表中剔除，所以`latency -t2`测试时你会发现没什么变化，还可能会变差了(最坏情况差不多一致，平均值变大了)，另外我们将linux中断affinity全都设置为CPU0处理，这些中断或多或少也会影响timer-IRQ的latency。

> 2021.5添加-- 最近发现xenomai内核定时器affinity为cpu0的问题已被社区修复。

三、软件方面
------

*   使用静态编译语言
    
*   常数时间复杂度算法
    
*   编写高性能的代码
    
*   尽量让分支有规律性，使用likely()/unlikely()或编写无分支代码
    
*   利用cache局部性原理，防止伪共享
    
*   合理分配任务优先级
    
*   尽量使用静态库
    
*   **避免实时任务中进行动态内存分配**
    
*   驱动程序中断处理尽可能短等等
    

四、优化结果对比
--------

笔者对以上各个条件配置前后对比过实时性改善效果，均有不同程度的优化效果，大家有兴趣也可自行测试。

> 以下结果基于 i5-7200U 8GB单通道DDR4,64GB emmc5.0，未使用RDT技术。

1-3 在已裁剪桌面下，压力加了内存。

#### 1\. 原始性能测试。

只使用了xenomai，CONFIG\_MIGRATION=n、CONFIG\_MCORE2=y\[x86\]、CONFIG\_PREEMPT=y、ACPI\_PROCESSOR =n\[x86\]、CONFIG\_CPU\_FREQ =n、CONFIG\_CPU\_IDLE =n。



优化项

配置与否

BISO

NO

Linux

NO

Full Dynamic Tick

NO

Offload RCU callback

NO

Full desktop

NO

stress

-c 10 -m 4

![](https://wsg-blogs-pic.oss-cn-beijing.aliyuncs.com/xenomai/raw_stress.png#crop=0&crop=0&crop=1&crop=1&id=A6Puc&originHeight=480&originWidth=640&originalType=binary&ratio=1&rotation=0&showTitle=false&status=done&style=none&title=)

#### 2\. 优化BIOS设置。

优化项

配置与否

BISO

**YES**

Linux

NO

Full Dynamic Tick

NO

Offload RCU callback

NO

Full desktop

NO

stress

-c 10 -m 4

![](https://wsg-blogs-pic.oss-cn-beijing.aliyuncs.com/xenomai/biso_stress.png#crop=0&crop=0&crop=1&crop=1&id=K2Ksi&originHeight=480&originWidth=640&originalType=binary&ratio=1&rotation=0&showTitle=false&status=done&style=none&title=)

#### 3\. Linux配置优化。

优化项

配置与否

BISO

YES

Linux

YES

Full Dynamic Tick

YES

Offload RCU callback

YES

Full desktop

NO

stress

-c 10 -m 4

![](https://wsg-blogs-pic.oss-cn-beijing.aliyuncs.com/xenomai/biso_linux_stress.png#crop=0&crop=0&crop=1&crop=1&id=A2Uk4&originHeight=480&originWidth=640&originalType=binary&ratio=1&rotation=0&showTitle=false&status=done&style=none&title=)

* * *

4-6 未添加内存压力

#### 4\. 裁剪桌面。

保留完整Ubuntu桌面前，且经**所有配置**：

优化项

配置与否

BISO

YES

Linux

YES

Full Dynamic Tick

YES

Offload RCU callback

YES

Full desktop

**NO—>YES**

stress

-c 10

![](https://wsg-blogs-pic.oss-cn-beijing.aliyuncs.com/xenomai/gui-cpu1.png#crop=0&crop=0&crop=1&crop=1&id=G6A6H&originHeight=480&originWidth=640&originalType=binary&ratio=1&rotation=0&showTitle=false&status=done&style=none&title=)

裁剪桌面后：

![](https://wsg-blogs-pic.oss-cn-beijing.aliyuncs.com/xenomai/cpu1.png#crop=0&crop=0&crop=1&crop=1&id=dKTQ4&originHeight=480&originWidth=640&originalType=binary&ratio=1&rotation=0&showTitle=false&status=done&style=none&title=)

裁剪前后对比：

![](https://wsg-blogs-pic.oss-cn-beijing.aliyuncs.com/xenomai/gui-and-nogui.png#crop=0&crop=0&crop=1&crop=1&id=XvCSZ&originHeight=480&originWidth=640&originalType=binary&ratio=1&rotation=0&showTitle=false&status=done&style=none&title=)

#### 5\. Full Dynamic Tick启用前后对比

裁剪桌面后，配置**cpu0未启用**Full Dynamic Tick，**cpu1启用**Full Dynamic Tick，加压与未加压对比。

优化项

配置与否

BISO

YES

Linux

YES

Full Dynamic Tick

CPU0:**NO**

;CPU1:**YES**

Offload RCU callback

CPU0==**NO**;CPU1:

**NO**==

Full desktop

**NO**

stress

-c 10

![](https://wsg-blogs-pic.oss-cn-beijing.aliyuncs.com/xenomai/cpu0.png#crop=0&crop=0&crop=1&crop=1&id=snD7N&originHeight=480&originWidth=640&originalType=binary&ratio=1&rotation=0&showTitle=false&status=done&style=none&title=)

![](https://wsg-blogs-pic.oss-cn-beijing.aliyuncs.com/xenomai/cpu1.png#crop=0&crop=0&crop=1&crop=1&id=LlLQY&originHeight=480&originWidth=640&originalType=binary&ratio=1&rotation=0&showTitle=false&status=done&style=none&title=)

对比cpu0与cpu1，最坏情况没有改善，但4us以上的latency改善明显。

![](https://wsg-blogs-pic.oss-cn-beijing.aliyuncs.com/xenomai/cpu0-1.png#crop=0&crop=0&crop=1&crop=1&id=ZPn0l&originHeight=480&originWidth=640&originalType=binary&ratio=1&rotation=0&showTitle=false&status=done&style=none&title=)

#### 6.桌面、rcu、tick前后比对

带桌面、未启用rcu\_nocb、未启用Full Dynamic Tick-------->裁桌面、启用rcu\_nocb、启用Full Dynamic Tick；

优化项

配置与否

BISO

YES            -->       YES

Linux

YES            -->       YES

Full Dynamic Tick

**NO --> YES**

Offload RCU callback

**NO-->YES**

Full desktop

YES - >NO

stress

-c 10

![](https://wsg-blogs-pic.oss-cn-beijing.aliyuncs.com/xenomai/anyay-nostress.png#crop=0&crop=0&crop=1&crop=1&id=MOkWb&originHeight=480&originWidth=640&originalType=binary&ratio=1&rotation=0&showTitle=false&status=done&style=none&title=)

#### 7.总对比

优化项

配置与否

BISO

NO            -->       **YES**

Linux

NO            -->       **YES**

Full Dynamic Tick

NO            -->       **YES**

Offload RCU callback

NO             -->       **YES**

Full desktop

YES             -->       **NO**

stress

-c 10 **-m 4**

![](https://wsg-blogs-pic.oss-cn-beijing.aliyuncs.com/xenomai/0-1.png#crop=0&crop=0&crop=1&crop=1&id=N2PCQ&originHeight=480&originWidth=640&originalType=binary&ratio=1&rotation=0&showTitle=false&status=done&style=none&title=)

四、实时性能测试
--------

### X86_64平台

下面直接给出最终的**应用空间任务Jitter**测试结果，使用的环境如下：

CPU

intel 赛扬 3865U[@1.8GHZ](/1.8GHZ)

Kernel

Linux 4.4.200

操作系统

Ubuntu 16.04

内存

8GB DDR3-1600 双通道

存储

64GB EMMC

测试条件：在stress压力下测试，同时一个QT应用程序绘制2维曲线图，QT CPU占用率99%。

    stress -c 10 -m 4


测试时间：211:04:55  
测试命令：

    latency -t0 -p 100 -P 99 -h -g result.txt


测试应用空间程序，优先级99，任务周期100us，测试结果输出到文件result.txt。经过接近10天的测试后，文件result.txt中latency分布结果如下：

    # 211:04:55 (periodic user-mode task, 100 us period, priority 99)
    # ----lat min|----lat avg|----lat max|-overrun|---msw|
    #       0.343|      1.078|     23.110|       0|     0|
    # Xenomai version: Xenomai/cobalt v3.1
    # Linux 4.4.200-xeno 
    ......
    # I-pipe releagese #20 detected
    # Cobalt core 3.1 detected
    # Compiler: gcc version 5.4.0 20160609 (Ubuntu 5.4.0-6ubuntu1~16.04.12) 
    # Build args: --enable-smp --enable-pshared --enable-tls 
    PKG_CONFIG_PATH=:/usr/xenomai/lib/pkgconfig:/usr/xenomai/lib/pkgconfig0 1
    0.5 1599357037
    1.5 1621130106
    2.5 56618753
    3.5 4386985
    4.5 3848531
    5.5 3556704
    6.5 3353649
    7.5 3033218
    8.5 2560133
    9.5 2035075
    10.5 1516866
    11.5 1038989
    12.5 680815
    13.5 417124
    14.5 224296
    15.5 115165
    16.5 58075
    17.5 27669
    18.5 11648
    19.5 4648
    20.5 1646
    21.5 467
    22.5 38
    23.5 1


其中第一列数据表示latency的值，第二列表示该值与上一个值之间这个范围的latency出现的次数，最小0.343us，平均latency 1.078us，最大23.110us。可见xenomai的实时性还是挺不错的。  
以上只是xenomai应用空间任务的实时性表现，如果使用内核空间任务会更好。**当然这只能说明操作系统能提供的实时性能，具体的还要看应用程序的设计等。**

此外，该测试基于X86平台，X86处理器的实时性与BIOS有很大关系，通常BIOS配置CPU具有更高的吞吐量，例如超线程、电源管理、CPU频率等，毕竟BIOS不是普通开发者能接触到的，如果能让BIOS对CPU针对实时系统配置的话，实时性会更好。如下图所示，平均抖动几乎在100纳秒以内。  
![](https://wsg-blogs-pic.oss-cn-beijing.aliyuncs.com/xenomai/tuyuitgyui%20-%201.png#crop=0&crop=0&crop=1&crop=1&id=vMwsi&originHeight=675&originWidth=1191&originalType=binary&ratio=1&rotation=0&showTitle=false&status=done&style=none&title=)

### ARM64平台(瑞芯微RK3588)

内核参数配置前后对比，GPU、NPU工作正常，同等压力下：  
![](https://wsg-blogs-pic.oss-cn-beijing.aliyuncs.com/xenomai/rk35881.png)

![](https://wsg-blogs-pic.oss-cn-beijing.aliyuncs.com/xenomai/Screenshot%20from%202023-10-21%2004-47-56.png)

相关链接
----

[https://www.intel.com/content/dam/www/public/us/en/documents/white-papers/cache-allocation-technology-white-paper.pdf](https://www.intel.com/content/dam/www/public/us/en/documents/white-papers/cache-allocation-technology-white-paper.pdf)

作者：[wsg1100](http://www.cnblogs.com/wsg1100/)

出处：[http://www.cnblogs.com/wsg1100/](http://www.cnblogs.com/wsg1100/)

本文版权归作者和博客园共有，欢迎转载，但必须给出原文链接，并保留此段声明，否则保留追究法律责任的权利。

分类: [xenomai3](https://www.cnblogs.com/wsg1100/category/1744176.html)

标签: [实时性相关](https://www.cnblogs.com/wsg1100/tag/%E5%AE%9E%E6%97%B6%E6%80%A7%E7%9B%B8%E5%85%B3/), [实时性能](https://www.cnblogs.com/wsg1100/tag/%E5%AE%9E%E6%97%B6%E6%80%A7%E8%83%BD/)

[好文要顶](javascript:void(0);) [关注我](javascript:void(0);) [收藏该文](javascript:void(0);) [微信分享](javascript:void(0);)

[![](https://pic.cnblogs.com/face/1250138/20200807212239.png)](https://home.cnblogs.com/u/wsg1100/)

[沐多](https://home.cnblogs.com/u/wsg1100/) ![](https://assets.cnblogs.com/vip.png "博客园VIP会员")  
[粉丝 \- 212](https://home.cnblogs.com/u/wsg1100/followers/) [关注 \- 7](https://home.cnblogs.com/u/wsg1100/followees/)  
[会员号：84](https://cnblogs.vip/)

[+加关注](javascript:void(0);)

10

0

[升级成为会员](https://cnblogs.vip/)

currentDiggType = 0;


[»](https://www.cnblogs.com/wsg1100/p/12822346.html) 下一篇： [【原创】xenomai内核解析之嵌入式实时linux概述](https://www.cnblogs.com/wsg1100/p/12822346.html "发布于 2020-05-03 16:27")

posted @ 2020-04-19 11:22  [沐多](https://www.cnblogs.com/wsg1100)  阅读(17210)  评论(19)    [收藏](javascript:void(0))  [举报](javascript:void(0))

var cb\_entryId = 12730720, cb\_entryCreatedDate = '2020-04-19 11:22', cb\_postType = 1, cb\_postTitle = '【原创】有利于提高xenomai /PREEMPT-RT 实时性的一些配置建议'; var allowComments = true, cb\_blogId = 385777, cb\_blogApp = 'wsg1100', cb\_blogUserGuid = '7e999067-b854-4fb5-ff22-08d4ef52ecb5'; mermaidRender.render() markdown\_highlight() zoomManager.apply("#cnblogs\_post\_body img:not(.code\_img\_closed):not(.code\_img\_opened)");

[刷新页面](#)[返回顶部](#top)

登录后才能查看或发表评论，立即 [登录](javascript:void(0);) 或者 [逛逛](https://www.cnblogs.com/) 博客园首页

[【推荐】100%开源！大型工业跨平台软件C++源码提供，建模，组态！](http://www.uccpsoft.com/index.htm)  
[【推荐】国内首个AI IDE，深度理解中文开发场景，立即下载体验Trae](https://www.trae.com.cn/?utm_source=advertising&utm_medium=cnblogs_ug_cpa&utm_term=hw_trae_cnblogs)  
[【推荐】Flutter适配HarmonyOS 5知识地图，实战解析+高频避坑指南](https://www.cnblogs.com/HarmonyOS5/p/18867837)  
[【推荐】凌霞软件回馈社区，携手博客园推出1Panel与Halo联合终身会员](https://www.cnblogs.com/cmt/p/18669224)  
[【推荐】轻量又高性能的 SSH 工具 IShell：AI 加持，快人一步](http://ishell.cc/)  

**相关博文：**  

·  [【原创】xenomai内核解析之xenomai初探](https://www.cnblogs.com/wsg1100/p/12833126.html "【原创】xenomai内核解析之xenomai初探")

·  [【xenomai3内核解析】系列文章大纲](https://www.cnblogs.com/wsg1100/p/13836497.html "【xenomai3内核解析】系列文章大纲")

·  [Xenomai (学习笔记)(转)](https://www.cnblogs.com/xihong2014/p/16574829.html "Xenomai (学习笔记)(转)")

·  [xenomai初探](https://www.cnblogs.com/uestc-mm/p/16995605.html "xenomai初探")

·  [Xenomai 再探](https://www.cnblogs.com/uestc-mm/p/17246221.html "Xenomai 再探")

**阅读排行：**  
· [垃圾qt，毁我青春](https://www.cnblogs.com/feiyangqingyun/p/18910173)  
· [开发十年现状之我的工作经历](https://www.cnblogs.com/hxjz/p/18910889)  
· [DeepSeek为什么现在感觉不火了？](https://www.cnblogs.com/skyell/p/18911342)  
· [一种更简单的方式运行 C# 代码，简化 C# 开发体验！](https://www.cnblogs.com/Can-daydayup/p/18909325)  
· [.NET 9中的异常处理性能提升分析：为什么过去慢，未来快](https://www.cnblogs.com/InCerry/p/-/dotnet-9-exception-pref-improve)   

### 常用链接

*   [我的随笔](https://www.cnblogs.com/wsg1100/p/ "我的博客的随笔列表")
*   [我的评论](https://www.cnblogs.com/wsg1100/MyComments.html "我的发表过的评论列表")
*   [我的参与](https://www.cnblogs.com/wsg1100/OtherPosts.html "我评论过的随笔列表")
*   [最新评论](https://www.cnblogs.com/wsg1100/comments "我的博客的评论列表")
*   [我的标签](https://www.cnblogs.com/wsg1100/tag/ "我的博客的标签列表")
*   [更多链接](#)

### [我的标签](https://www.cnblogs.com/wsg1100/tag/)

*   [xenomai(24)](https://www.cnblogs.com/wsg1100/tag/xenomai/)
*   [虚拟化(11)](https://www.cnblogs.com/wsg1100/tag/%E8%99%9A%E6%8B%9F%E5%8C%96/)
*   [linux(10)](https://www.cnblogs.com/wsg1100/tag/linux/)
*   [ubuntu(6)](https://www.cnblogs.com/wsg1100/tag/ubuntu/)
*   [实时性能(6)](https://www.cnblogs.com/wsg1100/tag/%E5%AE%9E%E6%97%B6%E6%80%A7%E8%83%BD/)
*   [EtherCAT(5)](https://www.cnblogs.com/wsg1100/tag/EtherCAT/)
*   [中断(5)](https://www.cnblogs.com/wsg1100/tag/%E4%B8%AD%E6%96%AD/)
*   [x86(4)](https://www.cnblogs.com/wsg1100/tag/x86/)
*   [RTIPC(4)](https://www.cnblogs.com/wsg1100/tag/RTIPC/)
*   [xenomai系统调用(3)](https://www.cnblogs.com/wsg1100/tag/xenomai%E7%B3%BB%E7%BB%9F%E8%B0%83%E7%94%A8/)
*   [更多](https://www.cnblogs.com/wsg1100/tag/)

### [随笔分类](https://www.cnblogs.com/wsg1100/post-categories)

*   [EtherCAT(7)](https://www.cnblogs.com/wsg1100/category/1935352.html)
*   [linux(17)](https://www.cnblogs.com/wsg1100/category/1744177.html)
*   [linux性能分析优化(3)](https://www.cnblogs.com/wsg1100/category/2206985.html)
*   [Linux中断子系统(3)](https://www.cnblogs.com/wsg1100/category/2439771.html)
*   [PREEMPT-RT(7)](https://www.cnblogs.com/wsg1100/category/2306468.html)
*   [X86(5)](https://www.cnblogs.com/wsg1100/category/1918095.html)
*   [xenomai3(36)](https://www.cnblogs.com/wsg1100/category/1744176.html)
*   [Xenomai4(1)](https://www.cnblogs.com/wsg1100/category/2280520.html)
*   [其他(14)](https://www.cnblogs.com/wsg1100/category/2206292.html)
*   [虚拟化(12)](https://www.cnblogs.com/wsg1100/category/2327130.html)

### 随笔档案

*   [2025年1月(6)](https://www.cnblogs.com/wsg1100/p/archive/2025/01)
*   [2024年12月(5)](https://www.cnblogs.com/wsg1100/p/archive/2024/12)
*   [2024年11月(3)](https://www.cnblogs.com/wsg1100/p/archive/2024/11)
*   [2024年10月(1)](https://www.cnblogs.com/wsg1100/p/archive/2024/10)
*   [2024年6月(2)](https://www.cnblogs.com/wsg1100/p/archive/2024/06)
*   [2024年5月(1)](https://www.cnblogs.com/wsg1100/p/archive/2024/05)
*   [2024年3月(1)](https://www.cnblogs.com/wsg1100/p/archive/2024/03)
*   [2024年2月(2)](https://www.cnblogs.com/wsg1100/p/archive/2024/02)
*   [2024年1月(1)](https://www.cnblogs.com/wsg1100/p/archive/2024/01)
*   [2023年7月(2)](https://www.cnblogs.com/wsg1100/p/archive/2023/07)
*   [2023年6月(1)](https://www.cnblogs.com/wsg1100/p/archive/2023/06)
*   [2023年5月(2)](https://www.cnblogs.com/wsg1100/p/archive/2023/05)
*   [2023年4月(1)](https://www.cnblogs.com/wsg1100/p/archive/2023/04)
*   [2023年3月(1)](https://www.cnblogs.com/wsg1100/p/archive/2023/03)
*   [2023年1月(2)](https://www.cnblogs.com/wsg1100/p/archive/2023/01)
*   [2022年9月(1)](https://www.cnblogs.com/wsg1100/p/archive/2022/09)
*   [2022年8月(8)](https://www.cnblogs.com/wsg1100/p/archive/2022/08)
*   [2022年4月(2)](https://www.cnblogs.com/wsg1100/p/archive/2022/04)
*   [2021年10月(1)](https://www.cnblogs.com/wsg1100/p/archive/2021/10)
*   [2021年6月(12)](https://www.cnblogs.com/wsg1100/p/archive/2021/06)
*   [2021年2月(1)](https://www.cnblogs.com/wsg1100/p/archive/2021/02)
*   [2021年1月(3)](https://www.cnblogs.com/wsg1100/p/archive/2021/01)
*   [2020年11月(2)](https://www.cnblogs.com/wsg1100/p/archive/2020/11)
*   [2020年10月(7)](https://www.cnblogs.com/wsg1100/p/archive/2020/10)
*   [2020年9月(1)](https://www.cnblogs.com/wsg1100/p/archive/2020/09)
*   [2020年8月(1)](https://www.cnblogs.com/wsg1100/p/archive/2020/08)
*   [2020年7月(7)](https://www.cnblogs.com/wsg1100/p/archive/2020/07)
*   [2020年6月(3)](https://www.cnblogs.com/wsg1100/p/archive/2020/06)
*   [2020年5月(4)](https://www.cnblogs.com/wsg1100/p/archive/2020/05)
*   [2020年4月(1)](https://www.cnblogs.com/wsg1100/p/archive/2020/04)
*   [更多](javascript:void(0))

### [阅读排行榜](https://www.cnblogs.com/wsg1100/most-viewed)

*   [1\. 【原创】xenomai3+linux构建linux实时操作系统-基于X86_64和arm(18046)](https://www.cnblogs.com/wsg1100/p/12864199.html)
*   [2\. 【原创】从Ubuntu-base构建ubuntu rootfs系统(以x86_64和arm为例)(17758)](https://www.cnblogs.com/wsg1100/p/13127636.html)
*   [3\. 【原创】EtherCAT主站IgH解析(一)--主站初始化、状态机与EtherCAT报文(17235)](https://www.cnblogs.com/wsg1100/p/14433632.html)
*   [4\. 【原创】有利于提高xenomai /PREEMPT-RT 实时性的一些配置建议(17210)](https://www.cnblogs.com/wsg1100/p/12730720.html)
*   [5\. 【原创】ubuntu下图形程序自启动的几种方法(13946)](https://www.cnblogs.com/wsg1100/p/12913491.html)

### [评论排行榜](https://www.cnblogs.com/wsg1100/most-commented)

*   [1\. 【原创】xenomai3+linux构建linux实时操作系统-基于X86_64和arm(28)](https://www.cnblogs.com/wsg1100/p/12864199.html)
*   [2\. 【原创】有利于提高xenomai /PREEMPT-RT 实时性的一些配置建议(19)](https://www.cnblogs.com/wsg1100/p/12730720.html)
*   [3\. 【原创】xenomai UDD介绍与UDD用户态驱动示例(12)](https://www.cnblogs.com/wsg1100/p/16610771.html)
*   [4\. 【原创】解BUG-xenomai内核与linux内核时间子系统之间存在漂移(9)](https://www.cnblogs.com/wsg1100/p/13662347.html)
*   [5\. 【原创】ARM64平台linux实时操作系统xenomai4(EVL)构建安装简述-基于rk3588(7)](https://www.cnblogs.com/wsg1100/p/18608339)

### [推荐排行榜](https://www.cnblogs.com/wsg1100/most-liked)

*   [1\. 【原创】为什么Linux不是实时操作系统(15)](https://www.cnblogs.com/wsg1100/p/17985934)
*   [2\. 【原创】xenomai内核解析之xenomai初探(12)](https://www.cnblogs.com/wsg1100/p/12833126.html)
*   [3\. 关于linux网桥(Linux Bridge)的一些个人记录(11)](https://www.cnblogs.com/wsg1100/p/18662571)
*   [4\. 【原创】有利于提高xenomai /PREEMPT-RT 实时性的一些配置建议(10)](https://www.cnblogs.com/wsg1100/p/12730720.html)
*   [5\. 【原创】波折重重：Linux实时系统Xenomai宕机问题的深度定位过程(8)](https://www.cnblogs.com/wsg1100/p/18631334)

### [最新评论](https://www.cnblogs.com/wsg1100/comments)

*   [1\. Re:【原创】RK3588/RK3576/RK3568/RK3562平台 IgH EthercAT主站编译安装及测试](https://www.cnblogs.com/wsg1100/p/18486644)
*   @7670 说的是基于这个分支开发，就是自己开发的...
*   --沐多
*   [2\. Re:【原创】RK3588/RK3576/RK3568/RK3562平台 IgH EthercAT主站编译安装及测试](https://www.cnblogs.com/wsg1100/p/18486644)
*   @写错了是主站代码没有dwmac-rk.c。那怎么在rk平台运行的？...
*   --7670
*   [3\. Re:【原创】RK3588/RK3576/RK3568/RK3562平台 IgH EthercAT主站编译安装及测试](https://www.cnblogs.com/wsg1100/p/18486644)
*   请问： 1、我使用非官方版本 ，并没有stmmac文件，和你描述的不太一样，而且最近修改是21年。还没有官方的新。所以使用--enable-dwmac-rk: 编译stmmac-rk网卡驱动报错，没有...
*   --7670
*   [4\. Re:【原创】ARM64平台linux实时操作系统xenomai4(EVL)构建安装简述-基于rk3588](https://www.cnblogs.com/wsg1100/p/18608339)
*   @沐多 请问，可以提供一份5.10的evl patch吗，感谢。我是基于rk3588的。...
*   --7670
*   [5\. Re:【原创】xenomai UDD介绍与UDD用户态驱动示例](https://www.cnblogs.com/wsg1100/p/16610771.html)
*   @morthlee 全志a40i...
*   --沐多

loadBlogSideColumn();

[博客园](https://www.cnblogs.com/)  ©  2004-2025  
[![](//assets.cnblogs.com/images/ghs.png)浙公网安备 33010602011771号](http://www.beian.gov.cn/portal/registerSystemInfo?recordcode=33010602011771) [浙ICP备2021040463号-3](https://beian.miit.gov.cn)## 目标


### 原理

