//
// Created by fzy on 23-11-9.
//
//
// Created by fzy on 23-7-5.
//
/** 使用说明：
 *      1. ./chat_server_udp 8000
 **/
#include "netflow/base/Logging.h"
#include "netflow/net//InetAddr.h"
#include "netflow/net/UDP/UdpServer.h"
#include "netflow/net/EventLoop.h"


#include <thread>
#include <string>
#include <cstdio>
#include <unistd.h>

using namespace netflow::base;
using namespace netflow::net;
using namespace std;
using namespace std::placeholders;

class UdpChatServer {
public:
    UdpChatServer(EventLoop* loop, const InetAddr& listenAddr)
            : server_(loop, listenAddr, "ChatServerUDP")
    {
        server_.setMessageCallback(std::bind(&UdpChatServer::onStringMessage, this, _1, _2, _3));
    }
    void start() {
        server_.start();
    }
private:
    void onStringMessage(const string& message, const InetAddr& remoteAddr, Timestamp receiveTime) {
        printf("<<< %s\n", message.c_str());
        server_.sendTo(message, remoteAddr);
    }
private:
    UdpServer server_;
};

int main(int argc, char* argv[]) {
    Logger::get().set_level(spdlog::level::info);
    STREAM_INFO << "current pid = " << getpid() << " current tid = " << this_thread::get_id();
    if (argc > 1) {
        EventLoop loop;
        uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
        InetAddr serverAddr(port);
        UdpChatServer server(&loop, serverAddr);
        server.start();
        loop.loop();
    }
    else {
        printf("Usage: %s port\n", argv[0]);
    }

}