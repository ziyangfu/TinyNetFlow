//
// Created by fzy on 2025/3/6.
//

#ifndef OSADAPTOR_PROCESS_CGROUPS_H
#define OSADAPTOR_PROCESS_CGROUPS_H

#include <filesystem>

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
};

}  // namespace osadaptor::process

#endif //OSADAPTOR_PROCESS_CGROUPS_H
