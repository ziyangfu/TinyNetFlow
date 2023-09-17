//
// Created by fzy on 23-8-28.
//

#ifndef TINYNETFLOW_SOMEIPSDMESSAGE_H
#define TINYNETFLOW_SOMEIPSDMESSAGE_H

#include <cstdint>

#include "src/someip/serviceDisvery/SdEntry.h"
#include "src/someip/serviceDisvery/SdOption.h"

namespace netflow::net::someip {
/*!
 * \brief SOME/IP-SD 消息 */
class SdMessage {

    struct SomeIpSdEntryHeader
    {
        uint8_t type_;
        uint8_t indexFirstOpts_;
        uint8_t indexSecondOpts_;
        uint8_t numberOfOpts;
    };

private:
    SdEntry entry_;
    SdOption option_;
};

}  // namespace netflow::net::someip


#endif //TINYNETFLOW_SOMEIPSDMESSAGE_H
