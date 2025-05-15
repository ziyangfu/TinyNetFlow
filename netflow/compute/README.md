目标：
针对嵌入式平台运行场景，以ARM64高性能平台为目标，封装/实现一个并行计算库，大概率是封装，没那个能力知道吧。
更倾向于ARM自家开发的并行计算的框架
更倾向于非CUDA，因为CUDA有自己的一套体系，如果要使用CADA，没必要自己去封装一层，直接用就
好了，还不如在tools或者哪里给出cuda的链接与教程呢。
主要关注推理，而且是推理的加速。而非训练，训练有云端专有硬件与平台去做这个事。

提供：
1. Eigen3.4,因为eigen已经基本完善，不怎么更新了，所以直接源码放在这里。
    对于ubuntu，可以直接安装：`sudo apt-get install libeigen3-dev`
2. 有向无环图
3. 并行计算库，基于OpenCL

- openCL、CUDA
- 高性能计算模块（GPU）
- 支持高耗费算力的算法
- 并行计算，异构计算

CPU密集型任务，走CPU计算
GPU计算型任务，走GPU计算通道，兼容CUDA，openCL

DAG有向无环图

腾讯图计算框架

1. 基于openCL的计算框架
2. 基于CUDA的计算框架，高级封装
3. 图计算

方向预留，用于提醒别忘记了

参考文档及库

1. [openCL封装](https://github.com/ProjectPhysX/OpenCL-Wrapper)
2. [CUDA 核心计算库 (CCCL)](https://github.com/nvidia/cccl)
3. [boost.compute](https://github.com/boostorg/compute)
4. [HIP](https://github.com/ROCm/hip)
5. [一些开源的 GPU 加速和并行计算库，涵盖了不同的编程语言和用途](https://www.cnblogs.com/suv789/p/18261810#%E9%80%9A%E7%94%A8%20GPU%20%E7%BC%96%E7%A8%8B%E5%BA%93)