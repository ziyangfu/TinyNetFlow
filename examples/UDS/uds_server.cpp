
/** 使用说明：
 *      1. 运行程序： ./uds_server
 **/
#include "netflow/base/Logging.h"
#include "netflow/net//InetAddr.h"
#include "netflow/IPC/UDS/UdsServer.h"
#include "netflow/net/EventLoop.h"

#include <thread>
#include <string>
#include <cstdio>
#include <unistd.h>

using namespace netflow::base;
using namespace netflow::net;
using namespace std;
using namespace std::placeholders;

class UdsChatServer {
public:
    UdsChatServer(EventLoop* loop)
            : server_(loop, "ChatServerUDP")
    {
        server_.setMessageCallback(std::bind(&UdsChatServer::onStringMessage, this, _1, _2));
    }
    void start() {
        server_.start();
    }
private:
    void onStringMessage(const string& message, Timestamp receiveTime) {
        STREAM_INFO << "message : " << message;
        server_.send(message);
    }
private:
    UdsServer server_;
};

int main(int argc, char* argv[]) {
    Logger::get().set_level(spdlog::level::trace);
    STREAM_INFO << "current pid = " << getpid() << " current tid = " << this_thread::get_id();
    EventLoop loop;
    UdsChatServer server(&loop);
    server.start();
    loop.loop();
}