//
// Created by fzy on 23-7-6.
//

#ifndef TINYNETFLOW_MQTTPROTOCOL_H
#define TINYNETFLOW_MQTTPROTOCOL_H

#include <cstdlib>

namespace netflow::net::mqtt {

#define DEFAULT_MQTT_PACKAGE_MAX_LENGTH 255
#define MQTT_DEFAULT_KEEPALIVE 60

#define DEFAULT_MQTT_PORT   1883

#define MQTT_PROTOCOL_V31   3
#define MQTT_PROTOCOL_V311  4
#define MQTT_PROTOCOL_V5    5

#define MQTT_PROTOCOL_NAME      "MQTT"
#define MQTT_PROTOCOL_NAME_v31  "MQIsdp"

/*
* connect flags
* 0        1               2       3-4         5           6           7
* reserved clean_session has_will will_qos will_retain has_password has_username
*/
#define MQTT_CONN_CLEAN_SESSION 0x02
#define MQTT_CONN_HAS_WILL      0x04
#define MQTT_CONN_WILL_RETAIN   0x20
#define MQTT_CONN_HAS_PASSWORD  0x40
#define MQTT_CONN_HAS_USERNAME  0x80

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

enum MqttConnAck{
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

int mqttHeadPack(MqttHead* head, u_int8_t buf[]);
int mqttHeadUnpack(MqttHead* head, const u_int8_t* buf, int len);

}  // namespace netflow::net::mqtt

#endif //TINYNETFLOW_MQTTPROTOCOL_H
