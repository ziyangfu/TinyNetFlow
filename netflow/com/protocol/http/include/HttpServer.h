#ifndef COM_PROTOCOL_HTTP_HTTP_SERVER_H
#define COM_PROTOCOL_HTTP_HTTP_SERVER_H

#include "IO/net/TcpServer.h"
#include <functional>
#include <string>

namespace com {

class HttpRequest;
class HttpResponse;
/** noncopyable */
class HttpServer {
public:
    /** HttpResponse 为什么是指针？ 不能引用？ */
    using HttpCallback = std::function< void (const HttpRequest&, HttpResponse*)>;
    HttpServer(osadaptor::net::EventLoop* loop, const osadaptor::net::InetAddr& listenAddr,
               const std::string& name,
               osadaptor::net::TcpServer::Option option = osadaptor::net::TcpServer::Option::kNoReusePort);

    osadaptor::net::EventLoop* getLoop() const { return server_.getLoop(); }

    void setHttpCallback(const HttpCallback& cb) { httpCallback_ = cb; }

    void setThreadNum(int numThreads) {
        server_.setThreadNum(numThreads);
    }

    void start();
private:
    void onConnection(const osadaptor::net::TcpConnectionPtr& conn);
    void onMessage(const osadaptor::net::TcpConnectionPtr& conn,
                   osadaptor::net::Buffer* buf, osadaptor::time::Timestamp receiveTime);
    void onRequest(const osadaptor::net::TcpConnectionPtr& conn, const HttpRequest& req);
private:
    osadaptor::net::TcpServer server_;
    HttpCallback httpCallback_;

};
} // namespace com



#endif //COM_PROTOCOL_HTTP_HTTP_SERVER_H
