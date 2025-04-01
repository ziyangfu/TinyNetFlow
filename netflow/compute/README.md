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