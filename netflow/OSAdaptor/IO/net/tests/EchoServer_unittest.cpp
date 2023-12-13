//
// Created by fzy on 23-6-14.
//

#include "netflow/OSAdaptor/IO/net/TcpServer.h"
#include "netflow/OSAdaptor/IO/reactor/EventLoop.h"
#include "netflow/OSAdaptor/IO/net/InetAddr.h"

#include "netflow/Log/Logging.h"

#include <thread>
#include <utility>
#include <string>

#include <stdio.h>
#include <unistd.h>

using namespace netflow::base;
using namespace netflow::net;
using namespace std;

using namespace std::placeholders;

int numThreads = 0;

class EchoServer
{
public:
    EchoServer(EventLoop* loop, const InetAddr& listenAddr)
            : loop_(loop),
              server_(loop, listenAddr, "EchoServer")
    {
        server_.setConnectionCallback(
                std::bind(&EchoServer::onConnection, this, _1));
        server_.setMessageCallback(
                std::bind(&EchoServer::onMessage, this, _1, _2, _3));
        server_.setThreadNum(numThreads);
    }

    void start()
    {
        server_.start();
    }
    // void stop();

private:
    void onConnection(const TcpConnectionPtr& conn)
    {
        STREAM_INFO << conn->getPeerAddr().toIpPort() << " -> "
                  << conn->getLocalAddr().toIpPort() << " is "
                  << (conn->isConnected() ? "UP" : "DOWN");

        conn->send("hello\n");
    }

    void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time)
    {
        string msg(buf->retrieveAllAsString());
        STREAM_TRACE << conn->getName() << " recv " << msg.size() << " bytes at " << time.toString();
        if (msg == "exit\n")
        {
            conn->send("bye\n");
            conn->shutdown();
        }
        if (msg == "quit\n")
        {
            loop_->quit();
        }
        conn->send(msg);
    }

    EventLoop* loop_;
    TcpServer server_;
};

int main(int argc, char* argv[])
{
    STREAM_INFO << "pid = " << getpid() << ", tid = " << std::this_thread::get_id();
    STREAM_INFO << "sizeof TcpConnection = " << sizeof(TcpConnection);
    if (argc > 1)
    {
        numThreads = atoi(argv[1]);
    }
    bool ipv6 = argc > 2;
    EventLoop loop;
    InetAddr listenAddr(2000, false, ipv6);
    EchoServer server(&loop, listenAddr);

    server.start();

    loop.loop();
}

