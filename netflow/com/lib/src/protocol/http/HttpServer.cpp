#include "protocol/http/HttpServer.h"
#include "protocol/http/HttpRequest.h"
#include "protocol/http/HttpResponse.h"
#include "protocol/http/HttpContext.h"

#include "spdlog/spdlog.h"
#include <any>

using namespace osadaptor::net;
using namespace com;
using namespace std::placeholders;

namespace com::detail {
    void defaultHttpCallback(const HttpRequest& request, HttpResponse* resp) {
        resp->setStatusCode(HttpResponse::k404NotFound);
        resp->setStatusMessage("Not Found");
        resp->setCloseConnection(true);
    }
} // namespace netflow::net::detail

HttpServer::HttpServer(osadaptor::net::EventLoop *loop,
                       const osadaptor::net::InetAddr &listenAddr,
                       const std::string &name,
                       TcpServer::Option option)
       : server_(loop, listenAddr, name, option),
         httpCallback_(detail::defaultHttpCallback)
{
    server_.setConnectionCallback(std::bind(&HttpServer::onConnection, this, _1));
    server_.setMessageCallback(std::bind(&HttpServer::onMessage, this, _1, _2, _3));
}

void HttpServer::start() {
    SPDLOG_INFO("HttpServer[{}] starts listening on {}", server_.getName(), server_.getIpPort());
    server_.start();
}

void HttpServer::onConnection(const osadaptor::net::TcpConnectionPtr &conn) {
    if (conn->isConnected()) {
        conn->setContext(HttpContext());
    }
}

void HttpServer::onMessage(const osadaptor::net::TcpConnectionPtr &conn, osadaptor::net::Buffer *buf,
                           osadaptor::time::Timestamp receiveTime) {
    HttpContext* context = std::any_cast<HttpContext>(conn->getMutableContext());
    if (!context->parseRequest(buf, receiveTime)) {
        conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
        conn->shutdown();
    }
    if (context->isGotAll()) {
        onRequest(conn, context->getRequest());
        context->reset();
    }
}

void HttpServer::onRequest(const osadaptor::net::TcpConnectionPtr &conn,
                           const HttpRequest &req) {
    const std::string& connection = req.getHeader("Connection");
    bool close = (connection == "close") ||
                 (req.getVersion() == HttpRequest::kHttp10 && connection != "Keep-Alive");
    HttpResponse response(close);
    httpCallback_(req, &response);
    Buffer buf;
    response.appendToBuffer(&buf);
    conn->send(&buf);
    if (response.isCloseConnection()) {
        conn->shutdown();
    }
}




