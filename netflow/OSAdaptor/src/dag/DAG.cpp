#include "dag/DAG.h"

/*!
 * \brief 采用 DFS 算法判断有向图是否存在环
 * */
bool Dag::IsCyclic(TaskNode* node, std::unordered_set<TaskNode*>& visited) const {
    if (visited.count(node)) return true;
    visited.insert(node);
    for (auto& dep : node->dependencies_) {
        if (HasCycle(dep, visited)) return true;
    }
    visited.erase(node);
    return false;
}

