// Watcher.cpp
#include "process/Watcher.h"
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>
#include <spdlog/spdlog.h>

namespace osadaptor::process {

Watcher& Watcher::getInstance() {
    static Watcher instance;
    return instance;
}

std::unordered_map<std::string, std::string> Watcher::parseProcFile(const std::string& path) {
    std::unordered_map<std::string, std::string> result;
    std::ifstream file(path);
    if (!file.is_open()) {
        SPDLOG_WARN("Failed to open file: {}", path);
        return result;
    }

    std::string line;
    while (std::getline(file, line)) {
        size_t pos = line.find(':');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            result[key] = value;
        }
    }
    return result;
}

std::vector<std::string> Watcher::readLinesFromFile(const std::string& path) {
    std::vector<std::string> lines;
    std::ifstream file(path);
    if (!file.is_open()) {
        SPDLOG_WARN("Failed to open file: {}", path);
        return lines;
    }

    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    return lines;
}

CpuUsage Watcher::getCpuUsage() {
    auto lines = readLinesFromFile("/proc/stat");
    for (const auto& line : lines) {
        if (line.rfind("cpu ", 0) == 0) {
            CpuUsage usage;
            std::istringstream iss(line);
            std::string name;
            iss >> name; // skip "cpu"
            iss >> usage.user >> usage.nice >> usage.system >> usage.idle;
            return usage;
        }
    }
    return {};
}

MemoryUsage Watcher::getMemoryUsage() {
    auto memInfo = parseProcFile("/proc/meminfo");
    MemoryUsage mem;
    mem.total = std::stoul(memInfo["MemTotal"]) * 1024;
    mem.free = std::stoul(memInfo["MemFree"]) * 1024;
    mem.buffers = std::stoul(memInfo["Buffers"]) * 1024;
    mem.cached = std::stoul(memInfo["Cached"]) * 1024;
    mem.available = std::stoul(memInfo["MemAvailable"]) * 1024;
    return mem;
}

std::vector<DiskIoStats> Watcher::getDiskIoStats() {
    std::vector<DiskIoStats> stats;
    auto lines = readLinesFromFile("/proc/diskstats");

    for (const auto& line : lines) {
        std::istringstream iss(line);
        std::string major, minor, device;
        uint64_t reads_completed, read_bytes, writes_completed, write_bytes;
        iss >> major >> minor >> device
            >> reads_completed >> read_bytes
            >> writes_completed >> write_bytes;

        stats.push_back({device, reads_completed, read_bytes, writes_completed, write_bytes});
    }
    return stats;
}

std::vector<NetworkStats> Watcher::getNetworkStats() {
    std::vector<NetworkStats> stats;
    auto lines = readLinesFromFile("/proc/net/dev");

    for (const auto& line : lines) {
        if (line.find(":") != std::string::npos) {
            std::istringstream iss(line);
            std::string interface;
            uint64_t rx_bytes, rx_packets, tx_bytes, tx_packets;
            iss >> interface
                >> rx_bytes >> rx_packets >> tx_bytes >> tx_packets;

            stats.push_back({interface, rx_bytes, rx_packets, tx_bytes, tx_packets});
        }
    }
    return stats;
}

ProcessStats Watcher::getProcessStats(pid_t pid) {
    ProcessStats stat;
    stat.pid = pid;

    std::string statPath = "/proc/" + std::to_string(pid) + "/stat";
    auto lines = readLinesFromFile(statPath);
    if (lines.empty()) return stat;

    std::istringstream iss(lines[0]);
    std::string token;
    std::vector<std::string> tokens;
    while (iss >> token) tokens.push_back(token);

    if (tokens.size() >= 22) {
        stat.cpu_usage = std::stod(tokens[13]) + std::stod(tokens[14]); // utime + stime
        stat.memory_kb = std::stoul(tokens[23]) * sysconf(_SC_PAGESIZE) / 1024; // RSS
        stat.virtual_memory_kb = std::stoul(tokens[22]) * sysconf(_SC_PAGESIZE) / 1024; // VSIZE
    }

    return stat;
}

ProcessStats Watcher::getCurrentProcessStats() {
    return getProcessStats(getpid());
}

void Watcher::enableEbpfMonitoring(bool enable) {
    useEbpf_ = enable;
    if (useEbpf_) {
        SPDLOG_INFO("eBPF monitoring is enabled.");
    } else {
        SPDLOG_INFO("eBPF monitoring is disabled.");
    }
}

} // namespace osadaptor::process








/*
 *
#include "process/Watcher.h"
#include <iostream>

int main() {
    using namespace osadaptor::process;

    auto& watcher = Watcher::getInstance();

    // 获取并打印 CPU 使用率
    auto cpu = watcher.getCpuUsage();
    std::cout << "CPU Usage: User=" << cpu.user
              << ", System=" << cpu.system
              << ", Idle=" << cpu.idle << "\n";

    // 获取内存使用情况
    auto mem = watcher.getMemoryUsage();
    std::cout << "Memory Total: " << mem.total / (1024 * 1024) << "MB\n";

    // 获取当前进程的资源使用
    auto procStat = watcher.getCurrentProcessStats();
    std::cout << "Current Process: PID=" << procStat.pid
              << ", Mem Usage=" << procStat.memory_kb << "KB\n";

    return 0;
}
 * */