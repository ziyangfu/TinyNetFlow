//
// Created by fzy on 23-7-6.
//

#include "MqttProtocol.h"

using namespace netflow::net::mqtt;

int mqttEstimateLength(MqttHead& head) {
    return 1 + 4 + static_cast<int>(head.length);
}

int mqttHeadPack(MqttHead* head, u_int8_t buf[]) {
    return 0;
}

int mqttHeadUnpack(MqttHead* head, const u_int8_t* buf, int len) {
    return 0;
}