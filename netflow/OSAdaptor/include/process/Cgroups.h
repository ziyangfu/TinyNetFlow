//
// Created by fzy on 2025/3/6.
//

#ifndef OSADAPTOR_PROCESS_CGROUPS_H
#define OSADAPTOR_PROCESS_CGROUPS_H

#include <cstdlib>

namespace oasadaptor::process {

typedef struct {
    char *name;
    char *path;
} Cgroup;
// 创建一个新的cgroup
int createCgroup(Cgroup *cg, const char *subsystem, const char *name);

// 删除一个cgroup
int deleteCgroup(Cgroup *cg, const char *subsystem);

// 将进程添加到cgroup
int addProcessToCgroup(pid_t pid, Cgroup *cg, const char *subsystem);

// 设置cgroup的内存限制
int setMemoryLimit(Cgroup *cg, const char *subsystem, unsigned long limit);

// 设置cgroup的CPU配额
int setCpuQuota(Cgroup *cg, const char *subsystem, long quota);

}  // namespace oasadaptor::process

#endif //OSADAPTOR_PROCESS_CGROUPS_H
