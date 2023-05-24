//
// Created by fzy on 23-5-24.
//

#include "TcpClient.h"

#include "EventLoop.h"
#include "Connector.h"

using namespace netflow::net;

TcpClient::TcpClient(netflow::net::EventLoop *loop, const netflow::net::InetAddr &serverAddr,
                     const std::string &name)
    : loop_(loop),
      name_(name),

 {}

 TcpClient::~TcpClient() {

}

void TcpClient::connect() {

}

void TcpClient::disconnect() {


}

void TcpClient::stop() {

}

/*!
 * \private ********************************************************************************************/
void TcpClient::newConnection(int sockfd) {

}

void TcpClient::removeConnection(const netflow::net::TcpConnectionPtr &conn) {

}