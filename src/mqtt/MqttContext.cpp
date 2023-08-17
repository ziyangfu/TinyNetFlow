//
// Created by fzy on 23-8-16.
//

#include "MqttContext.h"

using namespace netflow::net::mqtt;

MqttContext::MqttContext()
    : protocolVersion_(MQTT_PROTOCOL_V311),
      keepAlive_(MQTT_DEFAULT_KEEPALIVE),
      cleanSession(0),
      will_(nullptr),
      message_(nullptr)
{

}


MqttContext::~MqttContext() {

}

void MqttContext::swap(netflow::net::mqtt::MqttContext context) {

}


std::string MqttContext::printMqttContextToString() {

}