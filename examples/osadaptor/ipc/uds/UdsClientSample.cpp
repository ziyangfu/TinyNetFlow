//
// Created by fzy on 23-11-17.
//
/*!
 * \brief unix domain客户端
 *      ./uds_client
 * \fixme:
 *      BUG: 发送 hello world 会触发一个 TimeQueue 错误，如下：
        hello world
        [2025-01-10 15:16:16.124] [error] [TimerQueue.cpp:75] TimerQueue::handleRead() reads {} bytes instead of 8
        [2025-01-10 15:16:16.124] [info] [UdsClientSample.cpp:39] send message is : hello world
        [2025-01-10 15:16:16.125] [info] [UdsClientSample.cpp:45] receive message is : hello world
 *
 **/

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

using namespace std;
using namespace std::placeholders;

class UdsChatClient {
public:
    UdsChatClient(osadaptor::net::EventLoop* loop)
            : client_(loop, "UDSChatClient")
    {
        client_.setMessageCallback(
                std::bind(&UdsChatClient::onStringMessage, this, _1, _2));
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
    SPDLOG_INFO("start uds client, current tid {}", threadIdStr);

    osadaptor::net::EventLoopThread loopThread;

    UdsChatClient client(loopThread.startLoop());
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