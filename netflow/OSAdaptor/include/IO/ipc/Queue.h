//
// Created by fzy on 2025/1/8.
//

/*!
 *  实现一个IO队列，用于进程同步
 * */



/*!
说明
命名规范:

使用 k 前缀表示常量。
使用 Queue 和 QueueManager 类来表示队列和队列管理器。
使用 QueueElement 结构体来表示队列元素。
Queue 类:

包含 cap_, working_flag_, head_, tail_, queue_bytes_on_memory_ 成员变量。
提供 IsFull, IsEmpty, Size, Pop, Put, ConsumerIsWorking, MarkWorking, MarkNotWorking 方法。
QueueManager 类:

包含 path_, send_queue_, recv_queue_, mem_, mmap_map_type_, mem_fd_ 成员变量。
提供 CreateQueueManagerWithMemFd, CreateQueueManager, MappingQueueManagerMemfd, MappingQueueManager, Unmap 方法。
辅助函数:

CountQueueMemSize 函数计算队列内存大小。
IsArmArch 函数检查是否为 ARM 架构。
错误处理:

使用 std::runtime_error 来处理错误情况。
内存管理:

使用 std::vector<uint8_t> 来管理动态数组，确保内存安全。
使用 mmap 和 munmap 进行内存映射和解除映射。

 * */

#ifndef TINYNETFLOW_QUEUE_H
#define TINYNETFLOW_QUEUE_H

#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include <mutex>
#include <atomic>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <sys/syscall.h>
#include <linux/memfd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <iostream>
#include <algorithm>
#include <filesystem>

namespace shmipc {

constexpr size_t kQueueHeaderLength = 24;
constexpr size_t kQueueElementLen = 12;
constexpr size_t kQueueCount = 2;

enum class MemMapType {
    kDevShmFile,
    kMemFd
};

struct QueueElement {
    uint32_t seq_id;
    uint32_t offset_in_shm_buf;
    uint32_t status;
};

class Queue {
public:
    explicit Queue(uint32_t cap, uint8_t* data)
            : cap_(cap),
              working_flag_(reinterpret_cast<uint32_t*>(data + 4)),
              head_(reinterpret_cast<int64_t*>(data + 8)),
              tail_(reinterpret_cast<int64_t*>(data + 16)),
              queue_bytes_on_memory_(data + kQueueHeaderLength) {
        *head_ = 0;
        *tail_ = 0;
        *working_flag_ = 0;
    }

    bool IsFull() const {
        return Size() == cap_;
    }

    bool IsEmpty() const {
        return Size() == 0;
    }

    int64_t Size() const {
        return *tail_ - *head_;
    }

    std::tuple<QueueElement, std::runtime_error> Pop() {
        int64_t head = *head_;
        if (head >= *tail_) {
            return {{}, std::runtime_error("Queue is empty")};
        }
        size_t queue_offset = (head % cap_) * kQueueElementLen;
        QueueElement e;
        e.seq_id = *reinterpret_cast<uint32_t*>(queue_bytes_on_memory_ + queue_offset);
        e.offset_in_shm_buf = *reinterpret_cast<uint32_t*>(queue_bytes_on_memory_ + queue_offset + 4);
        e.status = *reinterpret_cast<uint32_t*>(queue_bytes_on_memory_ + queue_offset + 8);
        std::atomic_fetch_add(head_, 1);
        return {e, std::runtime_error("")};
    }

    std::runtime_error Put(const QueueElement& e) {
        std::lock_guard<std::mutex> lock(mutex_);
        int64_t tail = *tail_;
        if (tail - *head_ >= cap_) {
            return std::runtime_error("Queue is full");
        }
        size_t queue_offset = (tail % cap_) * kQueueElementLen;
        *reinterpret_cast<uint32_t*>(queue_bytes_on_memory_ + queue_offset) = e.seq_id;
        *reinterpret_cast<uint32_t*>(queue_bytes_on_memory_ + queue_offset + 4) = e.offset_in_shm_buf;
        *reinterpret_cast<uint32_t*>(queue_bytes_on_memory_ + queue_offset + 8) = e.status;
        std::atomic_fetch_add(tail_, 1);
        return std::runtime_error("");
    }

    bool ConsumerIsWorking() const {
        return *working_flag_ > 0;
    }

    bool MarkWorking() {
        return std::atomic_compare_exchange_weak(working_flag_, 0, 1);
    }

    bool MarkNotWorking() {
        *working_flag_ = 0;
        if (Size() == 0) {
            return true;
        }
        *working_flag_ = 1;
        return false;
    }

private:
    uint32_t cap_;
    uint32_t* working_flag_;
    int64_t* head_;
    int64_t* tail_;
    uint8_t* queue_bytes_on_memory_;
    std::mutex mutex_;
};

class QueueManager {
public:
    QueueManager(const std::string& path, std::shared_ptr<Queue> send_queue, std::shared_ptr<Queue> recv_queue,
                 std::vector<uint8_t> mem, MemMapType mmap_map_type, int mem_fd)
            : path_(path),
              send_queue_(send_queue),
              recv_queue_(recv_queue),
              mem_(mem),
              mmap_map_type_(mmap_map_type),
              mem_fd_(mem_fd) {}

    static std::shared_ptr<QueueManager> CreateQueueManagerWithMemFd(const std::string& queue_path_name, uint32_t queue_cap) {
        int mem_fd = memfd_create(queue_path_name.c_str(), 0);
        if (mem_fd == -1) {
            throw std::runtime_error("Failed to create memfd");
        }

        size_t mem_size = CountQueueMemSize(queue_cap) * kQueueCount;
        if (ftruncate(mem_fd, mem_size) != 0) {
            close(mem_fd);
            throw std::runtime_error("Failed to truncate share memory");
        }

        std::vector<uint8_t> mem(mem_size);
        if (mmap(mem.data(), mem_size, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, 0) == MAP_FAILED) {
            close(mem_fd);
            throw std::runtime_error("Failed to mmap");
        }
        std::memset(mem.data(), 0, mem_size);

        return std::make_shared<QueueManager>(
                queue_path_name, std::make_shared<Queue>(queue_cap, mem.data()), std::make_shared<Queue>(queue_cap, mem.data() + mem_size / 2),
                mem, MemMapType::kMemFd, mem_fd);
    }

    static std::shared_ptr<QueueManager> CreateQueueManager(const std::string& shm_path, uint32_t queue_cap) {
        std::filesystem::create_directories(std::filesystem::path(shm_path).parent_path());
        if (std::filesystem::exists(shm_path)) {
            throw std::runtime_error("Queue already exists");
        }

        size_t mem_size = CountQueueMemSize(queue_cap) * kQueueCount;
        int fd = open(shm_path.c_str(), O_CREAT | O_RDWR, 0666);
        if (fd == -1) {
            throw std::runtime_error("Failed to open file");
        }
        std::unique_ptr<FILE, decltype(&close)> file(fd, close);

        if (ftruncate(fd, mem_size) != 0) {
            throw std::runtime_error("Failed to truncate share memory");
        }

        std::vector<uint8_t> mem(mem_size);
        if (mmap(mem.data(), mem_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0) == MAP_FAILED) {
            throw std::runtime_error("Failed to mmap");
        }
        std::memset(mem.data(), 0, mem_size);

        return std::make_shared<QueueManager>(
                shm_path, std::make_shared<Queue>(queue_cap, mem.data()), std::make_shared<Queue>(queue_cap, mem.data() + mem_size / 2),
                mem, MemMapType::kDevShmFile, fd);
    }

    static std::shared_ptr<QueueManager> MappingQueueManagerMemfd(const std::string& queue_path_name, int mem_fd) {
        struct stat file_info;
        if (fstat(mem_fd, &file_info) != 0) {
            throw std::runtime_error("Failed to fstat");
        }

        size_t mapping_size = file_info.st_size;
        if (IsArmArch() && mapping_size % 16 != 0) {
            throw std::runtime_error("The memory size of queue should be a multiple of 16");
        }

        std::vector<uint8_t> mem(mapping_size);
        if (mmap(mem.data(), mapping_size, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, 0) == MAP_FAILED) {
            throw std::runtime_error("Failed to mmap");
        }

        return std::make_shared<QueueManager>(
                queue_path_name, std::make_shared<Queue>(mapping_size / 2, mem.data() + mapping_size / 2),
                std::make_shared<Queue>(mapping_size / 2, mem.data()), mem, MemMapType::kMemFd, mem_fd);
    }

    static std::shared_ptr<QueueManager> MappingQueueManager(const std::string& shm_path) {
        int fd = open(shm_path.c_str(), O_RDWR, 0666);
        if (fd == -1) {
            throw std::runtime_error("Failed to open file");
        }
        std::unique_ptr<FILE, decltype(&close)> file(fd, close);

        struct stat file_info;
        if (fstat(fd, &file_info) != 0) {
            throw std::runtime_error("Failed to fstat");
        }

        size_t mapping_size = file_info.st_size;
        if (IsArmArch() && mapping_size % 16 != 0) {
            throw std::runtime_error("The memory size of queue should be a multiple of 16");
        }

        std::vector<uint8_t> mem(mapping_size);
        if (mmap(mem.data(), mapping_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0) == MAP_FAILED) {
            throw std::runtime_error("Failed to mmap");
        }

        return std::make_shared<QueueManager>(
                shm_path, std::make_shared<Queue>(mapping_size / 2, mem.data() + mapping_size / 2),
                std::make_shared<Queue>(mapping_size / 2, mem.data()), mem, MemMapType::kDevShmFile, fd);
    }

    void Unmap() {
        if (munmap(mem_.data(), mem_.size()) != 0) {
            std::cerr << "QueueManager unmap error: " << strerror(errno) << std::endl;
        }
        if (mmap_map_type_ == MemMapType::kDevShmFile) {
            if (std::remove(path_.c_str()) != 0) {
                std::cerr << "QueueManager remove file: " << path_ << " failed, error=" << strerror(errno) << std::endl;
            } else {
                std::cout << "QueueManager remove file: " << path_ << std::endl;
            }
        } else {
            if (close(mem_fd_) != 0) {
                std::cerr << "QueueManager close queue fd: " << mem_fd_ << ", error=" << strerror(errno) << std::endl;
            } else {
                std::cout << "QueueManager close queue fd: " << mem_fd_ << std::endl;
            }
        }
    }

private:
    static size_t CountQueueMemSize(uint32_t queue_cap) {
        return kQueueHeaderLength + queue_cap * kQueueElementLen;
    }

    static bool IsArmArch() {
        // Implementation to check if the architecture is ARM
        return false;
    }

    std::string path_;
    std::shared_ptr<Queue> send_queue_;
    std::shared_ptr<Queue> recv_queue_;
    std::vector<uint8_t> mem_;
    MemMapType mmap_map_type_;
    int mem_fd_;
};

}  // namespace shmipc

int main() {
    try {
        auto queue_manager = shmipc::QueueManager::CreateQueueManager("/tmp/test_queue", 1024);
        // Example usage
        shmipc::QueueElement element{1, 2, 3};
        queue_manager->send_queue_->Put(element);

        auto [popped_element, err] = queue_manager->recv_queue_->Pop();
        if (!err) {
            std::cout << "Popped element: seq_id=" << popped_element.seq_id
                      << ", offset_in_shm_buf=" << popped_element.offset_in_shm_buf
                      << ", status=" << popped_element.status << std::endl;
        } else {
            std::cerr << "Error: " << err.what() << std::endl;
        }

        queue_manager->Unmap();
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}


#endif //TINYNETFLOW_QUEUE_H
