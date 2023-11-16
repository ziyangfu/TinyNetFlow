//
// Created by fzy on 23-11-15.
//
/** 使用说明：
 *      1. 运行程序1： ./multicast_server 224.0.0.2 8000
 *      2. 运行程序2： ./multicast_server 224.0.0.2 8000
 *      两者均加入 224.0.0.2 多播组， 并设置端口复用，因此客户端发送的消息，两个服务端程序均能接收到
 *      PS： 当程序1先开，接收几个消息后，再开程序2，此时再发送几条消息，当程序1退出组播组，且
 *           程序2还在组播组时，程序1还能接收到消息，感觉这是因为端口复用的缘故，在多主机环境下，应该不会有这问题
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

class UdpMulticastChatServer {
public:
    UdpMulticastChatServer(EventLoop* loop, const InetAddr& listenAddr, const InetAddr& multicastAddr)
            : server_(loop, listenAddr, "ChatServerUDP", UdpServer::Option::kReusePort),
              multicastAddr_(multicastAddr),
              count_(0)
    {
        server_.joinMulticastGroup(multicastAddr_);
        server_.setMessageCallback(std::bind(&UdpMulticastChatServer::onStringMessage, this, _1, _2, _3));
    }
    void start() {
        server_.start();
    }
private:
    void onStringMessage(const string& message, const InetAddr& remoteAddr, Timestamp receiveTime) {
        STREAM_INFO << "message : " << message;
        count_++;
        if (count_ == 5) {
            STREAM_INFO << "exit multicast group now, can not receive message later";
            server_.leaveMulticastGroup(multicastAddr_);
        }
        // server_.sendTo(message, remoteAddr);
    }
private:
    UdpServer server_;
    InetAddr multicastAddr_;
    int count_;
};

int main(int argc, char* argv[]) {
    Logger::get().set_level(spdlog::level::info);
    STREAM_INFO << "current pid = " << getpid() << " current tid = " << this_thread::get_id();
    if (argc > 2) {
        EventLoop loop;
        uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
        InetAddr localAddr(port);
        InetAddr multicastAddr(argv[1], port);  /** 设置协议族、IP地址与端口， 默认采用AF_INET */
        UdpMulticastChatServer server(&loop, localAddr, multicastAddr);
        server.start();
        loop.loop();
    }
    else {
        printf("Usage: %s multicast_addr port\n", argv[0]);
    }
}

/**
 * client：
        [2023-11-16 15:59:42.243] [info] [udpclient_multicast.cpp:64] pid = 21863
        123
        [2023-11-16 15:59:54.130] [info] [udpclient_multicast.cpp:46] send message is : 123
        456
        [2023-11-16 15:59:57.234] [info] [udpclient_multicast.cpp:46] send message is : 456
        789
        [2023-11-16 15:59:59.498] [info] [udpclient_multicast.cpp:46] send message is : 789
        1000
        [2023-11-16 16:00:02.459] [info] [udpclient_multicast.cpp:46] send message is : 1000
        245
        [2023-11-16 16:00:05.786] [info] [udpclient_multicast.cpp:46] send message is : 245
        900
        [2023-11-16 16:00:16.842] [info] [udpclient_multicast.cpp:46] send message is : 900 [这个消息，server没有接收]

server1:
    [2023-11-16 15:59:45.687] [info] [udpserver_multicast.cpp:55] current pid = 21865 current tid = 140656311269248
    [2023-11-16 15:59:54.130] [info] [udpserver_multicast.cpp:39] message : 123
    [2023-11-16 15:59:57.235] [info] [udpserver_multicast.cpp:39] message : 456
    [2023-11-16 15:59:59.498] [info] [udpserver_multicast.cpp:39] message : 789
    [2023-11-16 16:00:02.459] [info] [udpserver_multicast.cpp:39] message : 1000
    [2023-11-16 16:00:05.787] [info] [udpserver_multicast.cpp:39] message : 245
    [2023-11-16 16:00:05.787] [info] [udpserver_multicast.cpp:42] exit multicast group now

server2
    [2023-11-16 15:59:48.815] [info] [udpserver_multicast.cpp:55] current pid = 21866 current tid = 140155292321664
    [2023-11-16 15:59:54.130] [info] [udpserver_multicast.cpp:39] message : 123
    [2023-11-16 15:59:57.235] [info] [udpserver_multicast.cpp:39] message : 456
    [2023-11-16 15:59:59.498] [info] [udpserver_multicast.cpp:39] message : 789
    [2023-11-16 16:00:02.459] [info] [udpserver_multicast.cpp:39] message : 1000
    [2023-11-16 16:00:05.787] [info] [udpserver_multicast.cpp:39] message : 245
    [2023-11-16 16:00:05.787] [info] [udpserver_multicast.cpp:42] exit multicast group now

 */