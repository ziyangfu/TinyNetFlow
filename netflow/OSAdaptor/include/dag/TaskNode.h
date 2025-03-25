/*!
 * \brief DAG of task schedule
 * */

#ifndef TINYNETFLOW_TASKSCHEDULEDAG_H
#define TINYNETFLOW_TASKSCHEDULEDAG_H

#include <vector>
#include <unordered_map>

class TaskNode {
public:
    void AddSuccessor(TaskNode* node) { successors_.push_back(node); }
    // 其他成员函数...

private:
    std::vector<TaskNode*> successors_;
};

class TaskScheduleDag {
public:
    bool AddDependency(TaskNode* from, TaskNode* to);
    bool IsCyclic() const; // 环检测算法
    std::vector<TaskNode*> ScheduleOrder() const; // 拓扑排序

private:
    std::vector<TaskNode*> nodes_;
};

#endif //TINYNETFLOW_TASKSCHEDULEDAG_H
