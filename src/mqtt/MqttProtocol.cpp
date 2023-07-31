//
// Created by fzy on 23-7-6.
//

#include "MqttProtocol.h"

using namespace netflow::net::mqtt;

static int varintEncode(long long value, unsigned char* buf) {
    unsigned char ch;
    unsigned char *p = buf;
    int bytes = 0;
    do {
        ch = value & 0x7F;
        value >>= 7;
        *p++ = value == 0 ? ch : (ch | 0x80);
        ++bytes;
    } while (value);
    return bytes;
}

static int varintDecode(const unsigned char* buf, int* len) {
    long long ret = 0;
    int bytes = 0, bits = 0;
    const unsigned char *p = buf;
    do {
        if (len && *len && bytes == *len) {
            // Not enough length
            *len = 0;
            return 0;
        }
        ret |= ((long long)(*p & 0x7F)) << bits;
        ++bytes;
        if ((*p & 0x80) == 0) {
            // Found end
            if (len) *len = bytes;
            return ret;
        }
        ++p;
        bits += 7;
    } while(bytes < 10);

    // Not found end
    if (len) *len = -1;
    return ret;
}

int mqttEstimateLength(MqttHead& head) {
    /** header + 剩余长度（max 4 byte） + length */
    return 1 + 4 + static_cast<int>(head.length);
}
/*!
 * \brief 将MQTT头写入buffer */
int mqttHeadPack(MqttHead* head, unsigned char buf[]) {
    buf[0] = (head->type << 4) |
             (head->dup  << 3) |
             (head->qos  << 1) |
             (head->retain);
    /** 计算剩余长度的实际字节数，最大是4字节 */
    int bytes = varintEncode(head->length, buf + 1);
    return 1 + bytes;
}

int mqttHeadUnpack(MqttHead* head, const unsigned char* buf, int len) {
    head->type   = (buf[0] >> 4) & 0x0F;
    head->dup    = (buf[0] >> 3) & 0x01;
    head->qos    = (buf[0] >> 1) & 0x03;
    head->retain =  buf[0] & 0x01;
    int bytes = len - 1;
    head->length = varintDecode(buf + 1, &bytes);
    if (bytes <= 0) return bytes;
    return 1 + bytes;
}