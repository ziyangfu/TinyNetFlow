//
// Created by fzy on 23-7-6.
//

#ifndef TINYNETFLOW_MQTTPROTOCOL_H
#define TINYNETFLOW_MQTTPROTOCOL_H

#include <stdlib.h>

namespace netflow::net {

enum MqttType {
    MQTT_TYPE_CONNECT       = 1,
    MQTT_TYPE_CONNACK       = 2,
    MQTT_TYPE_PUBLISH       = 3,
    MQTT_TYPE_PUBACK        = 4,
    MQTT_TYPE_PUBREC        = 5,
    MQTT_TYPE_PUBREL        = 6,
    MQTT_TYPE_PUBCOMP       = 7,
    MQTT_TYPE_SUBSCRIBE     = 8,
    MQTT_TYPE_SUBACK        = 9,
    MQTT_TYPE_UNSUBSCRIBE   = 10,
    MQTT_TYPE_UNSUBACK      = 11,
    MQTT_TYPE_PINGREQ       = 12,
    MQTT_TYPE_PINGRESP      = 13,
    MQTT_TYPE_DISCONNECT    = 14,
};
enum MqttConnack {
    MQTT_CONNACK_ACCEPTED                       = 0,
    MQTT_CONNACK_REFUSED_PROTOCOL_VERSION       = 1,
    MQTT_CONNACK_REFUSED_IDENTIFIER_REJECTED    = 2,
    MQTT_CONNACK_REFUSED_SERVER_UNAVAILABLE     = 3,
    MQTT_CONNACK_REFUSED_BAD_USERNAME_PASSWORD  = 4,
    MQTT_CONNACK_REFUSED_NOT_AUTHORIZED         = 5,
};

struct MqttHead {
    unsigned char type:     4;
    unsigned char dup:      1;
    unsigned char qos:      2;
    unsigned char retain:   1;
    unsigned int  length;
};

struct MqttMessage {
    unsigned int    topic_len;
    const char*     topic;
    unsigned int    payload_len;
    const char*     payload;
    unsigned char   qos;
    unsigned char   retain;
};

const int64_t DEFAULT_MQTT_PACKAGE_MAX_LENGTH = 256; /** FIXME: no 256, is 246MB */

int mqttHeadPack();
int mqttHeadUnpack();

} // namespace netflow::net



#endif //TINYNETFLOW_MQTTPROTOCOL_H
