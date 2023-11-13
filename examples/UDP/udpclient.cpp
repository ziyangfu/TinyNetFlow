//
// Created by fzy on 23-11-9.
//
/** 使用说明：
 *      1. ./chat_client_udp localhost 8000
 **/

#include "netflow/base/Logging.h"
#include "netflow/net/UDP/UdpClient.h"
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

class UdpChatClient {
public:
    UdpChatClient(EventLoop* loop, const InetAddr& serverAddr)
    : client_(loop, serverAddr, "UDPChatClient")
    {
        client_.setMessageCallback(
                std::bind(&UdpChatClient::onStringMessage, this, _1, _2));
    }

    void connect()
    {
        client_.connect();
    }

    void write(const std::string& message)
    {
        client_.send(message);
    }
private:
    void onStringMessage(const string& message, Timestamp receiveTime) {
        printf("<<< %s\n", message.c_str());
    }
private:
    UdpClient client_;
    mutex mutex_;
};


int main(int argc, char* argv[])
{
    Logger::get().set_level(spdlog::level::trace);
    STREAM_INFO << "pid = " << getpid();
    if (argc > 2)
    {
        EventLoopThread loopThread;
        uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
        InetAddr serverAddr(argv[1], port);  /** 设置协议族、IP地址与端口， 默认采用AF_INET */

        UdpChatClient client(loopThread.startLoop(), serverAddr);
        client.connect();
        std::string line;
        while (std::getline(std::cin, line))
        {
            client.write(line);
        }
        this_thread::sleep_for(chrono::seconds(1)); // wait for disconnect, see ace/logging/client.cc
    }
    else
    {
        printf("Usage: %s host_ip port\n", argv[0]);
    }
}