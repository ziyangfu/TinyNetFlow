//
// Created by fzy on 23-8-16.
//

#include "MqttContext.h"
#include "src/base//Logging.h"

using namespace netflow::net::mqtt;

MqttContext::MqttContext()
    : protocolVersion_(MQTT_PROTOCOL_V311),
      keepAlive_(MQTT_DEFAULT_KEEPALIVE),
      cleanSession(0),
      will_(std::make_shared<MqttMessage>()),
      message_(std::make_shared<MqttMessage>()),
      clientId_(),
      userName_(),
      password_(),
      mid_(0)
{
}


MqttContext::~MqttContext() {

}

std::string MqttContext::printMqttContextToString() {

}

int MqttContext::mqttHeadPack(char *buf) {
    buf[0] = (head_.type << 4) |
             (head_.dup  << 3) |
             (head_.qos  << 1) |
             (head_.retain);
    /** 计算剩余长度的实际字节数，最大是4字节 */
    STREAM_INFO << "mqttHeadPack, head_.length == " << head_.length;
    int bytes = variateEncode(head_.length, buf + 1);
    return 1 + bytes;
}

int MqttContext::mqttHeadUnpack(const char *buf, int len) {
    head_.type   = (buf[0] >> 4) & 0x0F;
    head_.dup    = (buf[0] >> 3) & 0x01;
    head_.qos    = (buf[0] >> 1) & 0x03;
    head_.retain =  buf[0] & 0x01;
    int bytes = len - 1;
    head_.length = variateDecode(buf + 1, &bytes);
    if (bytes <= 0) return bytes;
    return 1 + bytes;
}

