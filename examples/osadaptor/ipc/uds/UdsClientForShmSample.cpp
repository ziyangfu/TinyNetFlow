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
    UdsChatClient(osadaptor::net::EventLoop* loop)
            : client_(loop, "UDSClientForShm")
    {
        client_.setMessageCallback(
                std::bind(&UdsChatClient::onStringMessage, this, _1, _2));
        client_.setConnectionCallback([](){
            SPDLOG_INFO("uds connect success");
        });
    }

    void connect()
    {
        client_.connect();
    }

    void write(const std::string& message)
    {
        SPDLOG_INFO("send message is : {}", message);
        unique_lock<mutex> lock(mutex_);
        client_.send(message);
    }
private:
    void onStringMessage(const string& message, osadaptor::time::Timestamp receiveTime) {
        SPDLOG_INFO("receive message is : {}", message);
    }
private:
    osadaptor::ipc::UdsClient client_;
    mutex mutex_;
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
    UdsChatClient client(loopThread.startLoop());
    client.connect();

    struct msghdr msg = {0};
    struct iovec iov[1];
    char buffer[1024];
    struct cmsghdr* cmsg;
    char control[CMSG_SPACE(sizeof(int))];

    // 创建memfd
    int memfd = memfd_create("netflow_osadaptor_shm_domian_20_port_20", 0);
    if (memfd == -1) {
        perror("memfd_create");
    }

    // 调整memfd大小
    if (ftruncate(memfd, kMemSize) == -1) {
        perror("ftruncate");
    }

    // 映射内存
    auto addr_mmap = mmap(NULL, kMemSize, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, 0);
    if (addr_mmap == MAP_FAILED) {
        perror("mmap");
        close(memfd);
        exit(EXIT_FAILURE);
    }
    // 写入数据到内存
    std::strcpy(static_cast<char*>(addr_mmap), "Hello from client");

    // 发送memfd
    iov[0].iov_base = buffer;
    iov[0].iov_len = 0;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    msg.msg_control = control;
    msg.msg_controllen = sizeof(control);

    cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(int));
    *((int*)CMSG_DATA(cmsg)) = memfd;

    msg.msg_controllen = cmsg->cmsg_len;

    std::string memfdMsg {msg};
    client.write(memfdMsg);

    if (sendmsg(client_fd, &msg, 0) == -1) {
        perror("sendmsg");
        close(memfd);
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    // 清理
    munmap(addr_mmap, kMemSize);
    close(memfd);


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