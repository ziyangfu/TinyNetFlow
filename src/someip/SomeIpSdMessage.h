//
// Created by fzy on 23-8-28.
//

#ifndef TINYNETFLOW_SOMEIPSDMESSAGE_H
#define TINYNETFLOW_SOMEIPSDMESSAGE_H

#include <cstdint>

/** 表示一条 SOME/IP-SD 消息 */
class SomeIpSdMessage {

    struct SomeIpSdEntryHeader
    {
        uint8_t type_;
        uint8_t indexFirstOpts_;
        uint8_t indexSecondOpts_;
        uint8_t numberOfOpts;   
    };
    struct SomeIpSdServiceEntryPayload
    {
        /* data */
    };

    struct SomeIpSdEventgroupEntryPayload
    {
        /* data */
    };

    struct SomeIpSdOptionHeader
    {
        /* data */
    };

    struct SomeIpSdOptionHeaderIpv4
    {
        /* data */
    };

    struct SomeIpSdOptionHeaderIpv6
    {
        /* data */
    };  

};


#endif //TINYNETFLOW_SOMEIPSDMESSAGE_H
