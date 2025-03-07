//
// Created by fzy on 2025/3/6.
//
#include "process/Cgroups.h"
#include "spdlog/spdlog.h"

#include <fstream>
#include <grp.h>

namespace osadaptor::process {

Cgroups::Cgroups(const std::string& subsystem, const std::string& name)
        : subsystem_(subsystem),
          name_(name),
          path_("/sys/fs/cgroup/" + subsystem + "/" + name)
{}

Cgroups::~Cgroups() {
    remove();
}

bool Cgroups::create() {
    try {
        std::filesystem::create_directory(path_);
        return true;
    } catch (const std::filesystem::filesystem_error& e) {
        std::string what = e.what();
        //SPDLOG_ERROR("Failed to create cgroup: {}", what);
        return false;
    }
}

bool Cgroups::remove() {
    try {
        if (std::filesystem::exists(path_)) {
            std::filesystem::remove_all(path_);
        }
        return true;
    } catch (const std::filesystem::filesystem_error& e) {
        SPDLOG_ERROR("Failed to delete cgroup");
        return false;
    }
}

bool Cgroups::addProcess(pid_t pid) {
    auto tasks_path = path_ / "tasks";
    return writeFile(tasks_path, std::to_string(pid));
}

bool Cgroups::setMemoryLimit(size_t limit) {
    auto memory_limit_path = path_ / "memory.limit_in_bytes";
    return writeFile(memory_limit_path, std::to_string(limit));
}

bool Cgroups::setCpuQuota(long quota) {
    auto cpu_quota_path = path_ / "cpu.cfs_quota_us";
    return writeFile(cpu_quota_path, std::to_string(quota));
}

bool Cgroups::writeFile(const std::filesystem::path& file_path, const std::string& content) {
    try {
        std::ofstream file(file_path);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file");
        }
        file << content;
        return true;
    } catch (const std::exception& e) {
        //std::cerr << "Failed to write to file: " << e.what() << std::endl;
        return false;
    }
}




} // namespace osadaptor::process