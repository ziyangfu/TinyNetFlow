//
// Created by fzy on 2024/12/17.
//
/*!
 * 共享内存的写端示例，往共享内存中写入数据，使用信号量通知客户端
 * */

#include "IO/ipc/shm/ShmWriter.h"
#include <string>
#include <thread>
#include <chrono>
#include "spdlog/spdlog.h"

int main(int argc, char** argv) {
    spdlog::set_level(spdlog::level::info);
    using ShmWriter = osadaptor::ipc::ShmWriter;
    ShmWriter shmWriter;
    shmWriter.start();
    std::string sendData {"this a message send via shm, from writer"};
    SPDLOG_INFO("send data: {}", sendData);
    while (true) {
        shmWriter.writeMessage(sendData);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
}