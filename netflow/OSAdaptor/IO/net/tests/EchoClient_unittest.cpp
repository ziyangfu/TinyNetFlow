//
// Created by fzy on 23-6-14.
//
/** 使用方法：
 *      1. 终端1： 先运行 ./netflow/net/tests/EchoServer_unittest
 *      2. 终端2： 后运行 ./netflow/net/tests/EchoClient_unittest localhost
 */

#include "netflow/OSAdaptor/IO/net/TcpClient.h"
#include "netflow/OSAdaptor/IO/reactor/EventLoop.h"
#include "netflow/OSAdaptor/IO/net/InetAddr.h"
#include "netflow/Log/Logging.h"

#include <thread>
#include <utility>
#include <functional>
#include <string>
#include <stdio.h>
#include <unistd.h>

using namespace netflow::base;
using namespace netflow::net;

using namespace std;
using namespace std::placeholders;

int numThreads = 0;
class EchoClient;
std::vector<std::unique_ptr<EchoClient>> clients;
int current = 0;

class EchoClient
{
public:
    EchoClient(EventLoop* loop, const InetAddr& listenAddr, const string& id)
            : loop_(loop),
              client_(loop, listenAddr, "EchoClient"+id)
    {
        client_.setConnectionCallback(std::bind(&EchoClient::onConnection, this, _1));
        client_.setMessageCallback(
                std::bind(&EchoClient::onMessage, this, _1, _2, _3));
        //client_.enableRetry();
    }

    void connect()
    {
        client_.connect();
    }
    // void stop();

private:
    void onConnection(const TcpConnectionPtr& conn)
    {
        STREAM_INFO << conn->getLocalAddr().toIpPort() << " -> "
                  << conn->getPeerAddr().toIpPort() << " is "
                  << (conn->isConnected() ? "UP" : "DOWN");

        if (conn->isConnected())
        {
            ++current;
            if (static_cast<size_t>(current) < clients.size())
            {
                clients[current]->connect();
            }
            STREAM_INFO << "*** connected " << current;
        }
        conn->send("world\n");
    }

    void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time)
    {
        string msg(buf->retrieveAllAsString());
        STREAM_INFO << conn->getName() << " recv " << msg.size() << " bytes at " << time.toString();
        if (msg == "quit\n")
        {
            conn->send("bye\n");
            conn->shutdown();
        }
        else if (msg == "shutdown\n")
        {
            loop_->quit();
        }
        else
        {
            conn->send(msg);
        }
    }

    EventLoop* loop_;
    TcpClient client_;
};

int main(int argc, char* argv[])
{
    Logger::get().set_level(spdlog::level::info);
    STREAM_INFO << "pid = " << getpid() << ", tid = " << this_thread::get_id();
    if (argc > 1)
    {
        EventLoop loop;
        bool ipv6 = argc > 3;
        InetAddr serverAddr(argv[1], 2000, ipv6);

        int n = 1;
        if (argc > 2)
        {
            n = atoi(argv[2]);
        }

        clients.reserve(n);
        for (int i = 0; i < n; ++i)
        {
            char buf[32];
            snprintf(buf, sizeof buf, "%d", i+1);
            clients.emplace_back(new EchoClient(&loop, serverAddr, buf));
        }

        clients[current]->connect();
        loop.loop();
    }
    else
    {
        printf("Usage: %s host_ip [current#]\n", argv[0]);
    }
}

