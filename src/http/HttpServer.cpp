//
// Created by fzy on 23-7-5.
//

#include "HttpServer.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "HttpContext.h"

#include "../base/Logging.h"

#include <any>

using namespace netflow::net;
using namespace std::placeholders;

namespace netflow::net::detail {
    void defaultHttpCallback(const HttpRequest& request, HttpResponse* resp) {
        resp->setStatusCode(HttpResponse::k404NotFound);
        resp->setStatusMessage("Not Found");
        resp->setCloseConnection(true);
    }
} // namespace netflow::net::detail

HttpServer::HttpServer(netflow::net::EventLoop *loop,
                       const netflow::net::InetAddr &listenAddr,
                       const std::string &name,
                       TcpServer::Option option)
       : server_(loop, listenAddr, name, option),
         httpCallback_(detail::defaultHttpCallback)
{
    server_.setConnectionCallback(std::bind(&HttpServer::onConnection, this, _1));
    server_.setMessageCallback(std::bind(&HttpServer::onMessage, this, _1, _2, _3));
}

void HttpServer::start() {
    STREAM_INFO << "HttpServer[" << server_.getName()
                << "] starts listening on " << server_.getIpPort();
    server_.start();
}

void HttpServer::onConnection(const netflow::net::TcpConnectionPtr &conn) {
    if (conn->isConnected()) {
        conn->setContext(HttpContext());
    }
}

void HttpServer::onMessage(const netflow::net::TcpConnectionPtr &conn, netflow::net::Buffer *buf,
                           base::Timestamp receiveTime) {
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

void HttpServer::onRequest(const netflow::net::TcpConnectionPtr &conn, const netflow::net::HttpRequest &req) {
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

