/*!
 * \brief task node 描述
 * */

#ifndef OSADAPTOR_DAG_TASK_NODE_H
#define OSADAPTOR_DAG_TASK_NODE_H

#include <functional>
#include <memory>
#include <vector>
#include <string>
#include <atomic>
#include <mutex>
#include <queue>
#include <unordered_map>
#include <future>

class TaskNode : public std::enable_shared_from_this<TaskNode> {
public:
    using Ptr = std::shared_ptr<TaskNode>;
    using WeakPtr = std::weak_ptr<TaskNode>;

    // 构造函数
    explicit TaskNode(std::string name, std::function<void()> task = nullptr)
            : taskName_(std::move(name)),
            task_(std::move(task))
            {}

    // 添加前置依赖任务
    void addDependency(WeakPtr dependency) {
        std::scoped_lock lock(mutex_);
        dependencies_.emplace_back(dependency);
        if (auto dep = dependency.lock()) {
            dep->addDependent(weak_from_this());
        }
    }

    // 获取所有前置任务
    std::vector<WeakPtr> getDependencies() const {
        std::scoped_lock lock(mutex_);
        return dependencies_;
    }

    // 获取所有后置任务
    std::vector<WeakPtr> getDependents() const {
        std::scoped_lock lock(mutex_);
        return dependents_;
    }

    // 检查是否就绪（所有前置任务已完成）
    bool isReady() const {
        std::scoped_lock lock(mutex_);
        for (const auto& dep : dependencies_) {
            if (auto d = dep.lock()) {
                if (!d->isCompleted()) return false;
            }
        }
        return true;
    }

    // 执行任务
    void execute() {
        if (!isReady()) return;

        if (task_) {
            task_();
        }

        std::scoped_lock lock(mutex_);
        is_completed_ = true;
    }

    // 状态查询
    bool isCompleted() const {
        std::scoped_lock lock(mutex_);
        return is_completed_;
    }

    // 获取任务ID
    std::string getTaskName() const {
        return taskName_;
    }

private:
    // 添加后置任务（内部使用）
    void addDependent(WeakPtr dependent) {
        std::scoped_lock lock(mutex_);
        dependents_.emplace_back(std::move(dependent));
    }

private:
    mutable std::mutex mutex_;
    std::string taskName_;
    std::vector<WeakPtr> dependencies_;   // 前置任务
    std::vector<WeakPtr> dependents_;     // 后置任务
    std::function<void()> task_;          // 任务函数
    bool is_completed_ = false;           // 完成状态
};

#endif //OSADAPTOR_DAG_TASK_NODE_H
