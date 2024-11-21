//
// Created by fzy on 23-7-6.
//
#include "src/mqtt/MqttProtocol.h"

using namespace netflow::net;

int mqtt::variateEncode(int64_t value, char *buf) {
    char ch;
    char *p = buf;
    int bytes = 0;
    do {
        ch = value & 0x7F;
        value >>= 7;
        *p++ = value == 0 ? ch : (ch | 0x80);
        ++bytes;
    } while (value);
    return bytes;
}

int mqtt::variateDecode(const char *buf, int *len) {
    int64_t ret = 0;
    int bytes = 0, bits = 0;
    const char *p = buf;
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
