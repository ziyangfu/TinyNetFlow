//
// Created by fzy on 23-6-14.
//
#include "netflow/Log/Logging.h"
#include "netflow/OSLayer/IO/reactor/EventLoop.h"
#include "netflow/OSLayer/IO/net/TcpClient.h"

#include <thread>
#include <chrono>

using namespace netflow::base;
using namespace netflow::net;


TcpClient* g_client;

void timeout()
{
    STREAM_INFO << "timeout";
    g_client->stop();
}

int main(int argc, char* argv[])
{
    EventLoop loop;
    InetAddr serverAddr("127.0.0.1", 2); // no such server
    TcpClient client(&loop, serverAddr, "TcpClient");
    g_client = &client;
    loop.runAfter(0.0, timeout);
    loop.runAfter(1.0, std::bind(&EventLoop::quit, &loop));
    client.connect();
    std::this_thread::sleep_for(std::chrono::microseconds(100 * 1000));
    loop.loop();
}