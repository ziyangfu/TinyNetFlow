//
// Created by fzy on 2025/1/8.
//

/**
 * 在构造函数中，选择使用uds协议还是shm共享内存
 * shm的文件创建，都是由client负责的
 * */

#ifndef TINYNETFLOW_IPCCLIENT_H
#define TINYNETFLOW_IPCCLIENT_H

#include <string>
#include <optional>

#include "IO/ipc/shm/ShmSysInterface.h"
#include "IO/ipc/shm/ShmConstant.h"
#include "IO/ipc/shm/ShmIdentifierInfo.h"
#include "IO/ipc/IpcMediaInfo.h"
#include "IO/ipc/internal/RingBuffer.h"
#include "IO/ipc/internal/IpcProtocolHeader.h"
#include "IO/ipc/uds/UdsClient.h"

#include "IO/reactor/EventLoop.h"

/**
 * 构造函数和析构函数：
ShmReader()：默认构造函数，初始化成员变量。
ShmReader(const std::string &sharedMemoryPath)：带参数的构造函数，初始化成员变量并设置共享内存路径。
~ShmReader()：析构函数，确保在对象销毁前关闭共享内存。
成员函数：
getSharedMemoryPath()：返回共享内存路径。
open()：打开共享内存文件并映射到内存。
start()：启动读取操作。
stop()：停止读取操作。
close()：关闭共享内存并清理资源。
readData(void* buffer, size_t bufferSize)：从共享内存读取数据到缓冲区，并使用信号量进行同步。
私有成员函数：
initSemaphore()：初始化信号量。
destroySemaphore()：销毁信号量。
成员变量：
sharedMemoryPath_：共享内存文件路径。
fd_：文件描述符。
mappedAddr_：映射的内存地址。
mappedSize_：映射的内存大小。
sem_：信号量指针。
isOpen_：指示共享内存是否已打开。
isRunning_：指示读取操作是否正在运行。
 */
namespace osadaptor::ipc {


class IpcClient {
public:
    explicit IpcClient(const IpcMediaInfo &info);
    ~IpcClient();
    const std::string &getSharedMemoryFilePath() const;
    bool connect();
    void disconnect();
    void close();
    size_t sendMessage(const std::string& message);
    void setConnectionCallback();
    void setMessageCallback();
private:
    net::EventLoop* loop_;
    int udsFd_;
    UdsClient udsClient_;
    internal::IpcProtocolHeader protocolHeader_;
    IpcMediaInfo mediaInfo_;
    std::optional<int> memFd_;
    std::optional<internal::RingBuffer> buffer_;
};

}  // namespace osadaptor::ipc


#endif //TINYNETFLOW_IPCCLIENT_H
