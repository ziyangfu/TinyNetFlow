
/** 使用说明：
 *      1. 运行程序： ./uds_server
 **/
#include "IO/net/InetAddr.h"
#include "IO/reactor/EventLoop.h"
#include "IO/ipc/uds/UdsServer.h"
#include "spdlog/spdlog.h"
#include <thread>
#include <string>
#include <sstream>


using namespace std;
using namespace std::placeholders;

class UdsChatServer {
public:
    UdsChatServer(osadaptor::net::EventLoop* loop)
            : server_(loop, "ChatServerUDS"),
              memFd_(-1),
              shmPtr_(nullptr)
    {
        server_.setMessageCallback(std::bind(&UdsChatServer::onStringMessage, this, _1, _2));
    }
    void start() {
        server_.start();
    }
private:
    void onStringMessage(const string& message, osadaptor::time::Timestamp receiveTime) {
        SPDLOG_INFO("message : {}", message);
        server_.send(message);
    }
private:
    osadaptor::ipc::UdsServer server_;
    int memFd_;
    std::uint8_t* shmPtr_;
};

int main(int argc, char* argv[]) {
    spdlog::set_level(spdlog::level::info);
    auto threadId = std::this_thread::get_id();
    std::ostringstream ss;
    ss << threadId;
    std::string threadIdStr = ss.str();
    SPDLOG_INFO("start uds server, current tid {}", threadIdStr);
    osadaptor::net::EventLoop loop;
    UdsChatServer server(&loop);
    server.start();
    loop.loop();
}