/*!
 * \brief DAG of task schedule
 * */
/** 这部分可以借鉴，或者直接导入 taskflow 开源库 */
/**
 * Linux软实时系统设计
 * 1. 低延迟通信，通信延迟
 *      1. DDS + TSN时间敏感网络实现跨端通信
 *      2. 零拷贝共享内存实现端内IPC通信
 * 2. 低延迟调度，调度延迟
用户空间层
├── DAG编排引擎（可视化配置、任务依赖解析） dag
├── 任务执行器（算子服务，通过LWP实现）  osadaptor-process
└── 监控模块（状态跟踪、异常重试）       trace， bpf

内核空间层
├── 实时调度策略（SCHED_FIFO/SCHED_RR）// RT调度策略
├── CPU隔离与绑定（cgroups/taskset）  // CPU亲和性设置
└　 中断优化（PREEMPT_RT补丁）        // RT-Linux补丁
 *
 * */
#ifndef OSADAPTOR_DAG_DAG_H
#define OSADAPTOR_DAG_DAG_H

#include "dag/TaskNode.h"
#include <vector>
#include <unordered_set>

/** 什么是拓扑排序？
 *  Kahn 算法
 *
 *  [拓扑排序之Kahn算法](https://zhuanlan.zhihu.com/p/644881066)
 *  [用C++ Workflow构建DAG，轻松实现高性能异步调度](https://zhuanlan.zhihu.com/p/690668258)
 *  [介绍一个Python的有向无环图任务编排库：Airflow](https://blog.csdn.net/lm33520/article/details/134459139)
 *  [taskflow](https://github.com/taskflow/taskflow/tree/master)
 *
 *  Topological Sorting 拓扑排序，指的是对于一个有向图而言，我们对所有顶点进行线性排序。
 *  要求对于有向图中任意一条由顶点u指向顶点v的边而言，排序结果中顶点u必须排在顶点v的前面。
 *  简单来说，拓扑排序就是将某集合上的偏序关系转换为该集合上的全序关系
 *  */

class Dag {
public:
    bool addEdge(TaskNode* from, TaskNode* to);
    int addNode(TaskNode* node);
    std::vector<TaskNode*> topologicSort() const; // 拓扑排序
    bool IsCyclic(TaskNode* node, std::unordered_set<TaskNode*>& visited) const; // 环检测算法
    void execute();
private:
    std::vector<TaskNode*> nodes_;
    std::unordered_map<int, std::vector<int>> adjacency_;
};

#endif //OSADAPTOR_DAG_DAG_H
