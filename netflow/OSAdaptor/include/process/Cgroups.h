/*!
 * \brief Linux Cgroups 资源控制管理
 * \details
 *        使用 cgroup V1 版本， 防止嵌入式Linux平台只有 cgroup V1
 *        对于 支持 cgroup V2 的 Linux平台， 请使用 CgroupV2Controller.h
 * */

#ifndef OSADAPTOR_PROCESS_CGROUPS_H
#define OSADAPTOR_PROCESS_CGROUPS_H

#include <filesystem>
#include <string>

namespace osadaptor::process {

class Cgroups {
public:
    Cgroups(const std::string& subsystem, const std::string& name);
    ~Cgroups();

    bool create();
    bool remove();
    bool addProcess(pid_t pid);
    bool setMemoryLimit(size_t limit);
    bool setCpuQuota(long quota);

private:
    std::string subsystem_;
    std::string name_;
    std::filesystem::path path_;

    bool writeFile(const std::filesystem::path& file_path, const std::string& content);
    const std::string cgroupBasePath_ {"/sys/fs/cgroup/unified/"};
    const std::string cpuQuota {""};
    const std::string memoryLimit {""};
    const std::string cgroupProcs {"cgroup.procs"};
};

}  // namespace osadaptor::process

#endif //OSADAPTOR_PROCESS_CGROUPS_H
