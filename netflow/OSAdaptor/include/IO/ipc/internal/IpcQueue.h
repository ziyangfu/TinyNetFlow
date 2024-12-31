//
// Created by fzy on 2024/12/18.
//

/**
 * 实现一个用于IPC的FIFO队列
 * */

#ifndef OSADAPTOR_IO_IPC_INTERNAL__IPC_QUEUE_H
#define OSADAPTOR_IO_IPC_INTERNAL__IPC_QUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

namespace internal {

class IpcQueue {
public:
    IpcQueue(const std::string& name, size_t size);
    ~IpcQueue();

    void enqueue(int data);
    int dequeue();

private:
    struct SharedQueue {
        std::mutex mtx;
        std::condition_variable cv;
        std::queue<int> queue;
    };

    SharedQueue* sharedQueue;
    int shm_fd;
    size_t shm_size;
};

IpcQueue::IpcQueue(const std::string& name, size_t size)
        : shm_size(size) {
    // 创建或打开共享内存对象
    shm_fd = shm_open(name.c_str(), O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        throw std::runtime_error("shm_open failed");
    }

    // 调整共享内存对象的大小
    if (ftruncate(shm_fd, shm_size) == -1) {
        close(shm_fd);
        throw std::runtime_error("ftruncate failed");
    }

    // 映射共享内存对象到进程地址空间
    sharedQueue = static_cast<SharedQueue*>(mmap(0, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0));
    if (sharedQueue == MAP_FAILED) {
        close(shm_fd);
        throw std::runtime_error("mmap failed");
    }

    // 初始化队列
    new (sharedQueue) SharedQueue();
}

IpcQueue::~IpcQueue() {
    // 销毁队列
    sharedQueue->~SharedQueue();

    // 取消映射共享内存对象
    if (munmap(sharedQueue, shm_size) == -1) {
        perror("munmap");
    }

    // 关闭共享内存对象
    if (close(shm_fd) == -1) {
        perror("close");
    }

    // 删除共享内存对象
    if (shm_unlink("/my_shm") == -1) {
        perror("shm_unlink");
    }
}

void IpcQueue::enqueue(int data) {
    std::unique_lock<std::mutex> lock(sharedQueue->mtx);
    sharedQueue->queue.push(data);
    lock.unlock();
    sharedQueue->cv.notify_one();
}

int IpcQueue::dequeue() {
    std::unique_lock<std::mutex> lock(sharedQueue->mtx);
    while (sharedQueue->queue.empty()) {
        sharedQueue->cv.wait(lock);
    }
    int data = sharedQueue->queue.front();
    sharedQueue->queue.pop();
    return data;
}

} // namespace internal

#endif //OSADAPTOR_IO_IPC_INTERNAL__IPC_QUEUE_H