//
// Created by fzy on 23-8-28.
//

#ifndef TINYNETFLOW_SOMEIPSDMESSAGE_H
#define TINYNETFLOW_SOMEIPSDMESSAGE_H

#include <cstdint>
#include <memory>

#include "netflow/someip/message/SomeIpHeader.h"  /** SOME/IP 消息头 */
#include "netflow/someip/someipd/serviceDiscovery/SdOption.h"
#include "netflow/someip/someipd/serviceDiscovery/SdEntry.h"

namespace netflow::net::someip {
/*!
 * \brief 描述一条 SOME/IP-SD 消息 */
class SdMessage {
public:
    SdMessage();
    ~SdMessage();

    struct SomeIpSdEntryHeader
    {
        uint8_t type_;
        uint8_t indexFirstOpts_;
        uint8_t indexSecondOpts_;
        uint8_t numberOfOpts;
    };

private:



private:
    SomeIpHeader header_;

    std::vector<std::unique_ptr<SdEntry>> entries_;
    std::vector<std::unique_ptr<SdOption>> options_;
};

}  // namespace netflow::net::someip


#endif //TINYNETFLOW_SOMEIPSDMESSAGE_H
