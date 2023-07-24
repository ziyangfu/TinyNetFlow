//
// Created by fzy on 23-7-6.
//

#include "MqttClient.h"

#include "../net/InetAddr.h"


using namespace netflow::net;
using namespace netflow::base;
using namespace std::placeholders;

MqttClient::MqttClient(netflow::net::EventLoop *loop, const netflow::net::InetAddr &serverAddr,
                       const std::string &name)
       : client_(loop, serverAddr, name)
{
    client_.setConnectionCallback(std::bind(&MqttClient::onConnection, this, _1));
    client_.setMessageCallback(std::bind(&MqttClient::onMessage, this, _1, _2, _3));
    client_.enableRetry();
}
/*!
 * \brief 建立TCP长链接 */
void MqttClient::connect() {
    client_.connect();
}

void MqttClient::disconnect() {
    client_.disconnect();
}
/*!
 * \brief 是否已经建立连接
 * \return true: 已经建立连接 */
bool MqttClient::isConnected() {
    return false;
}

/*!
 * \brief MQTT 发布 */
int MqttClient::publish(const std::string &topic, const std::string &payload, int qos, int retain,
                        netflow::net::MqttClient::MqttCallback ackCb) {

}


int MqttClient::subscribe(const char *topic, int qos, netflow::net::MqttClient::MqttCallback ackCb) {

}


int MqttClient::unSubscribe(const char *topic, netflow::net::MqttClient::MqttCallback ackCb) {


}




void MqttClient::onConnection(const netflow::net::TcpConnectionPtr &conn) {
    if (conn->isConnected()) {

    }

}

void MqttClient::onClose() {

}

void MqttClient::onMessage(const netflow::net::TcpConnectionPtr &conn, netflow::net::Buffer *buf,
                           base::Timestamp receiveTime) {

}