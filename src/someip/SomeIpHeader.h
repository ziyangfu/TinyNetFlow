//
// Created by fzy on 23-8-25.
//

#ifndef TINYNETFLOW_SOMEIPHEADER_H
#define TINYNETFLOW_SOMEIPHEADER_H

#include <cstdint>

class SomeIpHeader {
public:
    SomeIpHeader();
    /** 拷贝构造 */
    SomeIpHeader(const SomeIpHeader& header);
    ~SomeIpHeader();

    void packHeader();
    void unpackHeader();

private:
    uint16_t serviceId_;
    uint16_t methodId_;
    uint32_t length_;
    uint16_t clientId_;
    uint16_t sessionId_;
    uint8_t protocolVersion_;
    uint8_t interfaceVersion_;
    uint8_t messageType_;
    uint8_t returnCode_;

    uint32_t messageId_;
    uint32_t requestId_;
};


#endif //TINYNETFLOW_SOMEIPHEADER_H
