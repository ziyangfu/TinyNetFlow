//
// Created by fzy on 23-8-22.
//

#ifndef TINYNETFLOW_UDPSERVER_H
#define TINYNETFLOW_UDPSERVER_H

#include <thread>
#include <memory>
#include <vector>

namespace netflow::net {

class EventLoop;
class EventLoopThreadPool;

class UdpServer {
public:
    enum Status {
        kRunning = 1,
        kPaused = 2,
        kStopping = 3,
        kStopped = 4,
    };
    UdpServer();
    ~UdpServer();

    bool init(int port);
    bool init(std::vector<int>& ports);
    bool init(const std::string& listenPorts);   /** like "53,5353,1053"*/

    bool start();

    void stop();

    void pause();
    void toContinue();

    bool isRunning() const;
    bool isStopped() const;

    void setMessageCallback();

    void setEventLoopThreadPool(const std::shared_ptr<EventLoopThreadPool>& pool) {eventLoopThreadPool_ = pool; }

    void setReceiveBufferSize(size_t size) { receiveBufferSize = size; }

private:
    std::shared_ptr<EventLoopThreadPool> eventLoopThreadPool_;
    size_t receiveBufferSize;
    Status status_;
};
} // namespace netflow::net



#endif //TINYNETFLOW_UDPSERVER_H
