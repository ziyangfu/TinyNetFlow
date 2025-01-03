//
// Created by fzy on 2024/12/17.
//
/*!
 * 共享内存的读端示例
 * */

#include "IO/ipc/shm/ShmReader.h"
#include "spdlog/spdlog.h"
#include <string>
#include <thread>
#include <chrono>


int main(int argc, char** argv) {
    spdlog::set_level(spdlog::level::info);
    using ShmReader = osadaptor::ipc::ShmReader;
    std::string shmPath = "/tmp/shm_domain_01_port_01";
    ShmReader reader(shmPath);
    if (!reader.connect()) {
        SPDLOG_ERROR("reader connect writer failed");
    }
    std::string msg = reader.readMessage();
    SPDLOG_INFO("reader received message: {}", msg);
    return 0;
}