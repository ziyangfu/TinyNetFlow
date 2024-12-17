//
// Created by fzy on 23-11-15.
//
/*!
 * \brief UDP 组播演示
 * 使用说明：
 *      1. 运行程序： ./multicast_client 224.0.0.2 8000
 *      2. 输入字符串
 *      3. 服务端将接收到的客户端消息，返回给客户端 */



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

class UdpMulticastChatClient {
public:
    UdpMulticastChatClient(EventLoop* loop, const InetAddr& serverAddr)
            : client_(loop, serverAddr, "UDPChatClient")
    {
        client_.setMulticastLoop(true);
        client_.setMessageCallback(
                std::bind(&UdpMulticastChatClient::onStringMessage, this, _1, _2));
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
    UdpClient client_;
    mutex mutex_;
};


int main(int argc, char* argv[])
{
    Logger::get().set_level(spdlog::level::info);
    Logger::get().set_fatal_handle();
    STREAM_INFO << "pid = " << getpid();
    if (argc > 2)
    {
        EventLoopThread loopThread;
        uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
        InetAddr multicastAddr(argv[1], port);  /** 设置协议族、IP地址与端口， 默认采用AF_INET */

        UdpMulticastChatClient client(loopThread.startLoop(), multicastAddr);
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