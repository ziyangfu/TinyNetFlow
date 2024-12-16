//
// Created by fzy on 2024/12/16.
//


// generate by 豆包


#include "ShmSysInterface.h"
#include <iostream>

class ShmReader {
private:
    void* m_shm_ptr;  // 指向共享内存映射区域的指针
    int m_shm_size;  // 共享内存的大小
    int m_sem_id;    // 二元信号量的标识符

public:
    // 构造函数，用于初始化共享内存读端，传入共享内存大小和信号量标识符
    ShmReader(int shm_size, int sem_id);

    // 连接到共享内存并映射，获取共享内存的映射地址
    bool connect();

    // 从共享内存读取数据，参数data用于接收读取到的数据，len表示要读取的长度
    bool readData(void* data, size_t len);

    // 断开与共享内存的连接（取消映射）
    void disconnect();

    // 析构函数，释放相关资源
    ~ShmReader();
};


// 构造函数实现
ShmReader::ShmReader(int shm_size, int sem_id) : m_shm_size(shm_size), m_sem_id(sem_id), m_shm_ptr(nullptr) {}

// 连接到共享内存并映射
bool ShmReader::connect() {
    // 使用封装的函数进行共享内存映射
    m_shm_ptr = ShmSysInterface::mmap_shm(m_shm_size);
    if (m_shm_ptr == MAP_FAILED) {
        std::cerr << "Failed to mmap shared memory in reader." << std::endl;
        return false;
    }
    return true;
}

// 从共享内存读取数据
bool ShmReader::readData(void* data, size_t len) {
    if (m_shm_ptr == nullptr) {
        std::cerr << "Not connected to shared memory yet." << std::endl;
        return false;
    }
    // 先等待信号量，确保写端已经写入数据
    if (!ShmSysInterface::sem_wait(m_sem_id)) {
        std::cerr << "Failed to wait on semaphore in reader." << std::endl;
        return false;
    }
    // 进行数据拷贝（这里简单使用memcpy示例，实际可能根据数据结构做更复杂处理）
    memcpy(data, m_shm_ptr, len);
    // 释放信号量，让写端可以继续写入
    if (!ShmSysInterface::sem_post(m_sem_id)) {
        std::cerr << "Failed to post semaphore in reader." << std::endl;
        return false;
    }
    return true;
}

// 断开与共享内存的连接（取消映射）
void ShmReader::disconnect() {
    if (m_shm_ptr!= nullptr) {
        ShmSysInterface::munmap_shm(m_shm_ptr, m_shm_size);
        m_shm_ptr = nullptr;
    }
}

// 析构函数实现，释放资源
ShmReader::~ShmReader() {
    disconnect();
}



