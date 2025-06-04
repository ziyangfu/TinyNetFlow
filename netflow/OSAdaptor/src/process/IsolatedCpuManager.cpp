/*!
 * \brief 基于 Linux isolcpus 实现的 CPU 隔离管理器，
 *          预留部分CPU核心用于实时性任务
 * \file IsolatedCpuManager.cpp
 * */
#include "process/IsolatedCpuManager.h"

#include "process/ProcessConstants.h"
#include "BpfTrace.h"
#include "spdlog/spdlog.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <sys/sysinfo.h>
#include <sched.h>
#include <unistd.h>

namespace osadaptor::process {
/*!
 * \details
 *      注意：此操作通常只能在系统启动时通过内核参数完成
 *      也可以通过写入/sys/devices/system/cpu/isolated来动态调整（部分系统支持）
 * */
bool isolatedCpuManager::isolateCpus(const std::vector<int>& cpus) {
    // 使用 std::filesystem 检查文件是否存在
    std::filesystem::path isolatedPath(kIsolatedCpuSetDir);
    if (!std::filesystem::exists(isolatedPath)) {
        SPDLOG_ERROR("The file {} does not exist.", kIsolatedCpuSetDir);
        return false;
    }
    std::ofstream isolatedFile(kIsolatedCpuSetDir);
    if (!isolatedFile.is_open()) {
        SPDLOG_ERROR("Failed to open {}", kIsolatedCpuSetDir);
        return false;
    }
    std::string cpuList;
    for (size_t i = 0; i < cpus.size(); ++i) {
        cpuList += std::to_string(cpus[i]);
        if (i != cpus.size() - 1) {
            cpuList += ",";
        }
    }
    isolatedFile << cpuList;
    BPF_USER_PROBE_1("osa_process", "isolate_cpu_set", cpus);
    isolatedFile.close();
    return true;
}

bool isolatedCpuManager::unsetIsolateCpus() {
    // 使用 std::filesystem 检查文件是否存在
    std::filesystem::path isolatedPath(kIsolatedCpuSetDir);
    if (!std::filesystem::exists(isolatedPath)) {
        SPDLOG_ERROR("The file {} does not exist.", kIsolatedCpuSetDir);
        return false;
    }
    std::ofstream isolatedFile(kIsolatedCpuSetDir);
    if (!isolatedFile.is_open()) {
        SPDLOG_ERROR("Failed to open {}", kIsolatedCpuSetDir);
        return false;
    }
    isolatedFile << "";
    BPF_USER_PROBE_1("osa_process", "isolate_cpu_unset", 0);
    isolatedFile.close();
    return true;
}

bool isolatedCpuManager::isSetIsolatedCpuToSystem() {
    bool result{false};
    std::filesystem::path isolatedPath(kIsolatedCpuSetDir);
    if (!std::filesystem::exists(isolatedPath)) {
        SPDLOG_ERROR("The file {} does not exist.", kIsolatedCpuSetDir);
        return false;
    }
    std::ifstream isolatedFile(kIsolatedCpuSetDir);
    if (!isolatedFile.is_open()) {
        SPDLOG_ERROR("Failed to open {}", kIsolatedCpuSetDir);
        return false;
    }
    std::string line;
    std::getline(isolatedFile, line);
    isolatedFile.close();
    /** 如果读取的内容为不空，则说明设置了隔离 */
    if (!line.empty()) {
        result = true;
    }
    if (result) {
        SPDLOG_DEBUG("Current isolated CPUs: {}", line);
    } else {
        SPDLOG_DEBUG("No isolated CPUs are currently set.");
    }
    return result;
}

const std::vector<int> isolatedCpuManager::getIsolatedCpusFromSystem() {
    std::vector<int> result {};

    std::filesystem::path isolatedPath(kIsolatedCpuSetDir);
    if (!std::filesystem::exists(isolatedPath)) {
        SPDLOG_ERROR("The file {} does not exist.", kIsolatedCpuSetDir);
    }
    std::ifstream isolatedFile(kIsolatedCpuSetDir);
    if (!isolatedFile.is_open()) {
        spdlog::error(fmt::format("Failed to open {}", kIsolatedCpuSetDir));
        return {};
    }

    std::string line;
    std::getline(isolatedFile, line);
    isolatedFile.close();

    if (line.empty()) {
        SPDLOG_WARN("No isolated CPUs found in the system.");
        return result;
    }

    std::istringstream ss(line);
    std::string token;

    while (std::getline(ss, token, ',')) {
        try {
            int cpu = std::stoi(token);
            result.push_back(cpu);
        } catch (const std::exception& e) {
            SPDLOG_WARN("Failed to parse CPU value: {}", token);
        }
    }
    SPDLOG_DEBUG("Isolated CPUs: {}", line);
    return result;
}



} // namespace osadaptor::process

