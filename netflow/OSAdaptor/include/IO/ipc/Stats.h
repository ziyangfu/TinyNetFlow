//
// Created by fzy on 2025/1/9.
//

/*!
说明
命名规范:

使用 k 前缀表示常量。
使用 Monitor 类来表示监控接口。
使用 Stats 类来表示统计信息。
使用 PerformanceMetrics, StabilityMetrics, ShareMemoryMetrics 结构体来表示不同类型的指标。
Monitor 类:

定义了纯虚函数 OnEmitSessionMetrics 和 Flush，用于监控会话指标和刷新指标。
Stats 类:

包含 alloc_shm_error_count_, fallback_write_count_, fallback_read_count_, event_conn_error_count_, queue_full_error_count_, recv_polling_event_count_, send_polling_event_count_, out_flow_bytes_, in_flow_bytes_, hot_restart_success_count_, hot_restart_error_count_ 成员变量。
提供 IncrementAllocShmErrorCount, IncrementFallbackWriteCount, IncrementFallbackReadCount, IncrementEventConnErrorCount, IncrementQueueFullErrorCount, IncrementRecvPollingEventCount, IncrementSendPollingEventCount, AddOutFlowBytes, AddInFlowBytes, IncrementHotRestartSuccessCount, IncrementHotRestartErrorCount 方法来更新统计信息。
提供 GetPerformanceMetrics, GetStabilityMetrics, GetShareMemoryMetrics 方法来获取不同类型的指标。
辅助函数:

通过 std::atomic 来确保统计信息的线程安全性。
错误处理:

使用 std::runtime_error 来处理错误情况。
内存管理:

使用 std::atomic 来管理原子操作，确保统计信息的线程安全性。
*/

#ifndef TINYNETFLOW_STATS_H
#define TINYNETFLOW_STATS_H

#include <cstdint>
#include <atomic>
#include <memory>
#include <iostream>

namespace shmipc {

class Session;

// Monitor could emit some metrics with periodically
class Monitor {
public:
    virtual ~Monitor() = default;

    // OnEmitSessionMetrics was called by Session with periodically.
    virtual void OnEmitSessionMetrics(const PerformanceMetrics& performance_metrics,
                                      const StabilityMetrics& stability_metrics,
                                      const ShareMemoryMetrics& share_memory_metrics,
                                      const Session* session) = 0;

    // Flush metrics
    virtual std::runtime_error Flush() = 0;
};

struct PerformanceMetrics {
    uint64_t receive_sync_event_count = 0;  // the SyncEvent count that session had received
    uint64_t send_sync_event_count = 0;     // the SyncEvent count that session had sent
    uint64_t out_flow_bytes = 0;            // the out flow in bytes that session had sent
    uint64_t in_flow_bytes = 0;             // the in flow in bytes that session had received
    uint64_t send_queue_count = 0;          // the pending count of send queue
    uint64_t receive_queue_count = 0;       // the pending count of receive queue
};

struct StabilityMetrics {
    uint64_t alloc_shm_error_count = 0;      // the error count of allocating share memory
    uint64_t fallback_write_count = 0;       // the count of the fallback data write to unix/tcp connection
    uint64_t fallback_read_count = 0;        // the error count of receiving fallback data from unix/tcp connection every period

    // the error count of unix/tcp connection
    // which usually happened in that the peer's process exit(crashed or other reason)
    uint64_t event_conn_error_count = 0;

    // the error count due to the IO-Queue(SendQueue or ReceiveQueue) is full
    // which usually happened in that the peer was busy
    uint64_t queue_full_error_count = 0;

    // current all active stream count
    uint64_t active_stream_count = 0;

    // the successful count of hot restart
    uint64_t hot_restart_success_count = 0;

    // the failed count of hot restart
    uint64_t hot_restart_error_count = 0;
};

struct ShareMemoryMetrics {
    uint64_t capacity_of_share_memory_in_bytes = 0;  // capacity of all share memory
    uint64_t all_in_used_share_memory_in_bytes = 0;  // current in-used share memory
};

class Stats {
public:
    Stats() = default;

    void IncrementAllocShmErrorCount() {
        alloc_shm_error_count_.fetch_add(1, std::memory_order_relaxed);
    }

    void IncrementFallbackWriteCount() {
        fallback_write_count_.fetch_add(1, std::memory_order_relaxed);
    }

    void IncrementFallbackReadCount() {
        fallback_read_count_.fetch_add(1, std::memory_order_relaxed);
    }

    void IncrementEventConnErrorCount() {
        event_conn_error_count_.fetch_add(1, std::memory_order_relaxed);
    }

    void IncrementQueueFullErrorCount() {
        queue_full_error_count_.fetch_add(1, std::memory_order_relaxed);
    }

    void IncrementRecvPollingEventCount() {
        recv_polling_event_count_.fetch_add(1, std::memory_order_relaxed);
    }

    void IncrementSendPollingEventCount() {
        send_polling_event_count_.fetch_add(1, std::memory_order_relaxed);
    }

    void AddOutFlowBytes(uint64_t bytes) {
        out_flow_bytes_.fetch_add(bytes, std::memory_order_relaxed);
    }

    void AddInFlowBytes(uint64_t bytes) {
        in_flow_bytes_.fetch_add(bytes, std::memory_order_relaxed);
    }

    void IncrementHotRestartSuccessCount() {
        hot_restart_success_count_.fetch_add(1, std::memory_order_relaxed);
    }

    void IncrementHotRestartErrorCount() {
        hot_restart_error_count_.fetch_add(1, std::memory_order_relaxed);
    }

    PerformanceMetrics GetPerformanceMetrics() const {
        PerformanceMetrics metrics;
        metrics.receive_sync_event_count = 0;  // Placeholder, need to be implemented
        metrics.send_sync_event_count = 0;     // Placeholder, need to be implemented
        metrics.out_flow_bytes = out_flow_bytes_.load(std::memory_order_relaxed);
        metrics.in_flow_bytes = in_flow_bytes_.load(std::memory_order_relaxed);
        metrics.send_queue_count = 0;          // Placeholder, need to be implemented
        metrics.receive_queue_count = 0;       // Placeholder, need to be implemented
        return metrics;
    }

    StabilityMetrics GetStabilityMetrics() const {
        StabilityMetrics metrics;
        metrics.alloc_shm_error_count = alloc_shm_error_count_.load(std::memory_order_relaxed);
        metrics.fallback_write_count = fallback_write_count_.load(std::memory_order_relaxed);
        metrics.fallback_read_count = fallback_read_count_.load(std::memory_order_relaxed);
        metrics.event_conn_error_count = event_conn_error_count_.load(std::memory_order_relaxed);
        metrics.queue_full_error_count = queue_full_error_count_.load(std::memory_order_relaxed);
        metrics.active_stream_count = 0;       // Placeholder, need to be implemented
        metrics.hot_restart_success_count = hot_restart_success_count_.load(std::memory_order_relaxed);
        metrics.hot_restart_error_count = hot_restart_error_count_.load(std::memory_order_relaxed);
        return metrics;
    }

    ShareMemoryMetrics GetShareMemoryMetrics() const {
        ShareMemoryMetrics metrics;
        metrics.capacity_of_share_memory_in_bytes = 0;  // Placeholder, need to be implemented
        metrics.all_in_used_share_memory_in_bytes = 0;  // Placeholder, need to be implemented
        return metrics;
    }

private:
    std::atomic<uint64_t> alloc_shm_error_count_{0};
    std::atomic<uint64_t> fallback_write_count_{0};
    std::atomic<uint64_t> fallback_read_count_{0};
    std::atomic<uint64_t> event_conn_error_count_{0};
    std::atomic<uint64_t> queue_full_error_count_{0};
    std::atomic<uint64_t> recv_polling_event_count_{0};
    std::atomic<uint64_t> send_polling_event_count_{0};
    std::atomic<uint64_t> out_flow_bytes_{0};
    std::atomic<uint64_t> in_flow_bytes_{0};
    std::atomic<uint64_t> hot_restart_success_count_{0};
    std::atomic<uint64_t> hot_restart_error_count_{0};
};

}  // namespace shmipc

int main() {
    shmipc::Stats stats;

    // Example usage
    stats.IncrementAllocShmErrorCount();
    stats.IncrementFallbackWriteCount();
    stats.IncrementFallbackReadCount();
    stats.IncrementEventConnErrorCount();
    stats.IncrementQueueFullErrorCount();
    stats.IncrementRecvPollingEventCount();
    stats.IncrementSendPollingEventCount();
    stats.AddOutFlowBytes(1024);
    stats.AddInFlowBytes(512);
    stats.IncrementHotRestartSuccessCount();
    stats.IncrementHotRestartErrorCount();

    auto performance_metrics = stats.GetPerformanceMetrics();
    auto stability_metrics = stats.GetStabilityMetrics();
    auto share_memory_metrics = stats.GetShareMemoryMetrics();

    std::cout << "PerformanceMetrics: "
              << "ReceiveSyncEventCount=" << performance_metrics.receive_sync_event_count
              << ", SendSyncEventCount=" << performance_metrics.send_sync_event_count
              << ", OutFlowBytes=" << performance_metrics.out_flow_bytes
              << ", InFlowBytes=" << performance_metrics.in_flow_bytes
              << ", SendQueueCount=" << performance_metrics.send_queue_count
              << ", ReceiveQueueCount=" << performance_metrics.receive_queue_count << std::endl;

    std::cout << "StabilityMetrics: "
              << "AllocShmErrorCount=" << stability_metrics.alloc_shm_error_count
              << ", FallbackWriteCount=" << stability_metrics.fallback_write_count
              << ", FallbackReadCount=" << stability_metrics.fallback_read_count
              << ", EventConnErrorCount=" << stability_metrics.event_conn_error_count
              << ", QueueFullErrorCount=" << stability_metrics.queue_full_error_count
              << ", ActiveStreamCount=" << stability_metrics.active_stream_count
              << ", HotRestartSuccessCount=" << stability_metrics.hot_restart_success_count
              << ", HotRestartErrorCount=" << stability_metrics.hot_restart_error_count << std::endl;

    std::cout << "ShareMemoryMetrics: "
              << "CapacityOfShareMemoryInBytes=" << share_memory_metrics.capacity_of_share_memory_in_bytes
              << ", AllInUsedShareMemoryInBytes=" << share_memory_metrics.all_in_used_share_memory_in_bytes << std::endl;

    return 0;
}

#endif //TINYNETFLOW_STATS_H
