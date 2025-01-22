//
// Created by fzy on 23-11-17.
//
/*!
 * \brief unix domain客户端
 *      ./uds_client */

#include "IO/reactor/EventLoopThread.h"
#include "IO/ipc/uds/UdsClient.h"
#include "spdlog/spdlog.h"

#include <string>
#include <mutex>
#include <iostream>
#include <thread>
#include <chrono>
#include <functional>
#include <sstream>

#include <sys/mman.h>

using namespace std;
using namespace std::placeholders;

constexpr int kMemSize = 64 * 1024;
class UdsChatClient {
public:
    UdsChatClient(osadaptor::net::EventLoop* loop, bool usingShm)
            : client_(loop, "UDSClientForShm"),
              memFd_(-1),
              shmPtr_(nullptr),
              isShm_(usingShm)
    {
        client_.setMessageCallback(
                std::bind(&UdsChatClient::onStringMessage, this, _1, _2));
        client_.setUdsConnectionCallback([this](){
            SPDLOG_INFO("uds connect success, start shm connecting");
            this->shmConnect();

        });
        client_.setShmConnectionCallback([](){
            SPDLOG_INFO("shm connect success");
        });

    }

    ~UdsChatClient() {
        close(memFd_);
    }

    void connect()
    {
        client_.connect();
    }

    void shmConnect() {
        /** shmClient的连接建立步骤
         * step1: client通过uds发送协议头
         * step2: 服务端收到uds的消息，发送确认消息
         * */
        createSharedMemory();
        std::string message {"0521043"};
        client_.sendMemFd(memFd_, message);
        std::string shmConnectionMsgStepOne {"test client for shared memory"};
        writeShmMessage(shmConnectionMsgStepOne);
    }

    void write(const std::string& message)
    {
        SPDLOG_INFO("send message via uds is : {}", message);
        unique_lock<mutex> lock(mutex_);
        client_.send(message);
    }

    void writeShmMessage(const std::string& message) {
        SPDLOG_INFO("send message via shm is : {}", message);
        /** 写入shared memory */
        std::memcpy(static_cast<void*>(shmPtr_), message.c_str(), message.size() + 1);
    }

    void readFromShm() {
        void* ptr = static_cast<void*>(shmPtr_);
        SPDLOG_INFO("Client read from shared memory: {}",  static_cast<char*>(ptr));
    }

    int getMemFd() {
        return memFd_;
    }
    std::uint8_t* getShmPtr() {
        return shmPtr_;
    }
private:
    void onStringMessage(const string& message, osadaptor::time::Timestamp receiveTime) {
        SPDLOG_INFO("receive message is : {}", message);
        if (isShm_) {
            readFromShm();
        }
    }

    void createSharedMemory() {
        // 创建memfd
        memFd_ = memfd_create("shared_mem", MFD_CLOEXEC);
        if (memFd_ == -1) {
            perror("memfd_create");
        }

        // 设置共享内存大小
        const size_t size = 4096;
        if (ftruncate(memFd_, size) == -1) {
            perror("ftruncate");
        }

        // 映射共享内存
        void* ptr = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, memFd_, 0);
        if (ptr == MAP_FAILED) {
            perror("mmap");
        }
        shmPtr_ = static_cast<std::uint8_t*>(ptr);
    }

private:
    osadaptor::ipc::UdsClient client_;
    mutex mutex_;
    int memFd_;
    std::uint8_t* shmPtr_;
    bool isShm_;
};

int main(int argc, char* argv[])
{
    spdlog::set_level(spdlog::level::info);
    auto threadId = std::this_thread::get_id();
    std::ostringstream ss;
    ss << threadId;
    std::string threadIdStr = ss.str();
    SPDLOG_INFO("start uds client for shm sync, current tid {}", threadIdStr);

    osadaptor::net::EventLoopThread loopThread;
    UdsChatClient client(loopThread.startLoop(), true);
    client.connect();

    std::string line;
    while (std::getline(std::cin, line))
    {
        client.write(line);
    }
    this_thread::sleep_for(chrono::seconds(1)); // wait for disconnect, see ace/logging/client.cc
    if (!strcmp(argv[1], "-h")) {
        printf("Usage: %s \n", argv[0]);
    }
}