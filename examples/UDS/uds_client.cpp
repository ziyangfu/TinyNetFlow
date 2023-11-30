//
// Created by fzy on 23-11-17.
//
/*!
 * \brief unix domain客户端
 *      ./uds_client */

#include "netflow/base/Logging.h"
#include "netflow/IPC/UDS/UdsClient.h"
#include "netflow/net/EventLoopThread.h"

#include <string>
#include <mutex>
#include <iostream>
#include <thread>
#include <chrono>
#include <functional>

using namespace netflow::base;
using namespace netflow::net;
using namespace std;
using namespace std::placeholders;

class UdsChatClient {
public:
    UdsChatClient(EventLoop* loop)
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
        STREAM_INFO << "send message is : " << message;
        unique_lock<mutex> lock(mutex_);
        client_.send(message);
    }
private:
    void onStringMessage(const string& message, Timestamp receiveTime) {
        printf("<<< %s\n", message.c_str());
    }

private:
    UdsClient client_;
    mutex mutex_;
};


int main(int argc, char* argv[])
{
    Logger::get().set_level(spdlog::level::trace);
    Logger::get().set_fatal_handle();
    STREAM_INFO << "pid = " << getpid();

    EventLoopThread loopThread;

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