/*!
 * \brief 资源监控器，基于 /proc 文件或 eBPF 读取系统资源信息
 * */

#ifndef OSADAPTOR_PROCESS_WATCHER_H
#define OSADAPTOR_PROCESS_WATCHER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>

#include "process/Types.h"

namespace osadaptor::process {

class Watcher {
public:
    // 获取单例实例
    static Watcher& getInstance();

    // 获取整体 CPU 使用率
    CpuUsage getCpuUsage();

    // 获取内存使用情况
    MemoryUsage getMemoryUsage();

    // 获取所有磁盘 I/O 统计
    std::vector<DiskIoStats> getDiskIoStats();

    // 获取所有网络接口统计
    std::vector<NetworkStats> getNetworkStats();

    // 获取指定进程的资源使用情况
    ProcessStats getProcessStats(pid_t pid);

    // 获取当前进程的资源使用情况
    ProcessStats getCurrentProcessStats();

    // 预留 eBPF 接口
    void enableEbpfMonitoring(bool enable);

private:
    Watcher() = default;
    ~Watcher() = default;

    Watcher(const Watcher&) = delete;
    Watcher& operator=(const Watcher&) = delete;

    // 内部辅助函数
    std::unordered_map<std::string, std::string> parseProcFile(const std::string& path);
    std::vector<std::string> readLinesFromFile(const std::string& path);

private:
    bool useEbpf_ = false;
    std::chrono::steady_clock::time_point lastCpuTime_;
    CpuUsage lastCpuUsage_;
};

} // namespace osadaptor::process

#endif //OSADAPTOR_PROCESS_WATCHER_H
