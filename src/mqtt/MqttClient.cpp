//
// Created by fzy on 23-7-6.
//

#include "MqttClient.h"

using namespace netflow::net;
using namespace netflow::base;

void MqttClient::onConnection(const netflow::net::TcpConnectionPtr &conn) {

}

void MqttClient::onMessage(const netflow::net::TcpConnectionPtr &conn, netflow::net::Buffer *buf,
                           base::Timestamp receiveTime) {

}