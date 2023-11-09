//
// Created by fzy on 23-7-5.
//
/** 使用说明：
 *      1. ./chat_client localhost 8000
 **/

#include "netflow/base/Logging.h"
#include "netflow/net/TcpClient.h"
#include "netflow/net/EventLoopThread.h"

#include "codec.h"

#include <string>
#include <mutex>
#include <iostream>
#include <thread>
#include <chrono>

using namespace netflow::base;
using namespace netflow::net;
using namespace std;
using namespace std::placeholders;

class ChatClient
{
public:
    ChatClient(EventLoop* loop, const InetAddr& serverAddr)
            : client_(loop, serverAddr, "ChatClient"),
              codec_(std::bind(&ChatClient::onStringMessage, this, _1, _2, _3))
    {
        client_.setConnectionCallback(
                std::bind(&ChatClient::onConnection, this, _1));
        client_.setMessageCallback(
                std::bind(&LengthHeaderCodec::onMessage, &codec_, _1, _2, _3));
        client_.enableRetry();
    }

    void connect()
    {
        client_.connect();
    }

    void disconnect()
    {
        client_.disconnect();
    }

    void write(const std::string& message)
    {
        unique_lock<mutex> lock(mutex_);
        if (connection_)
        {
            codec_.send(connection_.get(), message);
        }
    }

private:
    void onConnection(const TcpConnectionPtr& conn)
    {
        STREAM_INFO << conn->getPeerAddr().toIpPort() << "-> "
                    << conn->getLocalAddr().toIpPort() << " is "
                    << (conn->isConnected() ? "UP" : "DOWN");

        unique_lock<mutex> lock(mutex_);
        if (conn->isConnected())
        {
            connection_ = conn;
        }
        else
        {
            connection_.reset();
        }
    }

    void onStringMessage(const TcpConnectionPtr&,
                         const string& message,
                         Timestamp)
    {
        printf("<<< %s\n", message.c_str());
    }

    TcpClient client_;
    LengthHeaderCodec codec_;
    mutex mutex_;
    TcpConnectionPtr connection_;
};

int main(int argc, char* argv[])
{
    Logger::get().set_level(spdlog::level::info);
    STREAM_INFO << "pid = " << getpid();
    if (argc > 2)
    {
        EventLoopThread loopThread;
        uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
        InetAddr serverAddr(argv[1], port); // 设置协议族、IP地址与端口， 默认采用AF_INET

        ChatClient client(loopThread.startLoop(), serverAddr);
        client.connect();
        std::string line;
        while (std::getline(std::cin, line))
        {
            client.write(line);
        }
        client.disconnect();
        this_thread::sleep_for(chrono::seconds(1)); // wait for disconnect, see ace/logging/client.cc
    }
    else
    {
        printf("Usage: %s host_ip port\n", argv[0]);
    }
}
