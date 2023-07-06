//
// Created by fzy on 23-7-5.
//

#ifndef TINYNETFLOW_HTTPSERVER_H
#define TINYNETFLOW_HTTPSERVER_H

#include "../net/TcpServer.h"

#include <functional>
#include <string>

namespace netflow::net {

class HttpRequest;
class HttpResponse;
/** noncopyable */
class HttpServer {
public:
    /** HttpResponse 为什么是指针？ 不能引用？ */
    using HttpCallback = std::function< void (const HttpRequest&, const HttpResponse*)>;
    HttpServer(EventLoop* loop, const InetAddr& listenAddr, const std::string& name,
               TcpServer::Option option = TcpServer::kNoReusePort);

    EventLoop* getLoop() const { return server_.getLoop(); }

    void setHttpCallback(const HttpCallback& cb) { httpCallback_ = cb; }

    void setThreadNum(int numThreads) {
        server_.setThreadNum(numThreads);
    }

    void start();
private:
    void onConnection(const TcpConnectionPtr& conn);
    void onMessage(const TcpConnectionPtr& conn, Buffer* buf, base::Timestamp receiveTime);
    void onRequest(const TcpConnectionPtr& conn, const HttpRequest& req);
private:
    TcpServer server_;
    HttpCallback httpCallback_;

};
} // namespace netflow::net



#endif //TINYNETFLOW_HTTPSERVER_H
