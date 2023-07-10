//
// Created by fzy on 23-7-6.
//

#include "MqttClient.h"

#include "../net/InetAddr.h"


using namespace netflow::net;
using namespace netflow::base;

MqttClient::MqttClient(netflow::net::EventLoop *loop,
                       const netflow::net::InetAddr &listenAddr,
                       const std::string &name,
                       TcpServer::Option option)
        : server_(loop, listenAddr, name, option),
          version(MQTT_PROTOCOL_V311),
          port(DEFAULT_MQTT_PORT)
{}

void MqttClient::onConnection(const netflow::net::TcpConnectionPtr &conn) {
    if (conn->isConnected()) {

    }

}

void MqttClient::onMessage(const netflow::net::TcpConnectionPtr &conn, netflow::net::Buffer *buf,
                           base::Timestamp receiveTime) {

}