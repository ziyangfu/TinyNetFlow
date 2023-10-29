//
// Created by fzy on 23-7-5.
//
/** 使用说明：
 *      1. ./chat_server 8000
 **/
#include "../../src/base/Logging.h"
#include "../../netflow/net//TcpServer.h"
#include "../../netflow/net//EventLoop.h"
#include "../../netflow/net//InetAddr.h"

#include "codec.h"

#include <thread>
#include <set>
#include <string>
#include <cstdio>
#include <unistd.h>

using namespace netflow::base;
using namespace netflow::net;
using namespace std;
using namespace std::placeholders;

class ChatServer {
public:
    ChatServer(EventLoop* loop, const InetAddr& listenAddr)
        : server_(loop, listenAddr, "ChatServer"),
          codec_(std::bind(&ChatServer::onStringMessage, this, _1, _2, _3))
    {
        server_.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1));
        server_.setMessageCallback(std::bind(&LengthHeaderCodec::onMessage, &codec_, _1, _2, _3));
    }
    void start() {
        server_.start();
    }
private:
    void onConnection(const TcpConnectionPtr& conn) {
        STREAM_INFO << conn->getPeerAddr().toIpPort() << "-> "
                    << conn->getLocalAddr().toIpPort() << " is "
                    << (conn->isConnected() ? "UP" : "DOWN");
        if (conn->isConnected()) {
            connections_.insert(conn);
        }
        else {
            connections_.erase(conn);
        }
    }
    void onStringMessage(const TcpConnectionPtr&, const std::string& message, Timestamp receiveTime){
        for (auto conn : connections_) {
            codec_.send(conn.get(), message);  /** 回显 */
        }
    }

    using ConnectionList = std::set<TcpConnectionPtr>;
    TcpServer server_;
    LengthHeaderCodec codec_;
    ConnectionList connections_;
};

int main(int argc, char* argv[]) {
    Logger::get().set_level(spdlog::level::info);
    STREAM_INFO << "current pid = " << getpid() << " current tid = " << this_thread::get_id();
    if (argc > 1) {
        EventLoop loop;
        uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
        InetAddr serverAddr(port);
        ChatServer server(&loop, serverAddr);
        server.start();
        loop.loop();
    }
    else {
        printf("Usage: %s port\n", argv[0]);
    }

}