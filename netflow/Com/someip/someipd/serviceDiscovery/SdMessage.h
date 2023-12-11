//
// Created by fzy on 23-8-28.
//

#ifndef TINYNETFLOW_SOMEIPSDMESSAGE_H
#define TINYNETFLOW_SOMEIPSDMESSAGE_H

#include <cstdint>
#include <memory>
#include <array>
#include <vector>

#include "netflow/Com/someip/message/SomeIpHeader.h"  /** SOME/IP 消息头 */
#include "SdOption.h"
#include "SdEntry.h"

namespace netflow::net::someip {
/*!
 * \brief 描述一条 SOME/IP-SD 消息
 *
 *  * SOME/IP SD message :
 *    +-----------------------------+
 *    |   SOME/IP Header            |
 *    +-----------------------------+
 *    |   SOME/IP SD Header         |
 *    +-----------------------------+
 *    |   Entries Length Field      |
 *    +-----------------------------+
 *    |   List of Entries           |
 *    +-----------------------------+
 *    |   Options Length Field      |
 *    +-----------------------------+
 *    |   List of Options           |
 *    +-----------------------------+
 *
 *    */

class SdEntry;
class SdOption;
class SdEventGroupEntry;
class SdServiceEntry;
class SdOptionIpv4;
class SdOptionIpv6;

class SdMessage {
public:
    SdMessage();
    ~SdMessage();

    SomeIpLength getLength() const;
    void setLength(SomeIpLength len);

    SomeIpSdFlag getFlag() const;
    void setFlag(SomeIpSdFlag flag);

    std::shared_ptr<SdEventGroupEntry>  createEventGroupEntry();
    std::shared_ptr<SdServiceEntry>  createServiceEntry();

    SdOptionIpv4 createOptionIpv4();
    SdOptionIpv6 createOptionIpv6();

private:
    SomeIpHeader header_;
    /** SD header 部分 */
    SomeIpSdFlag flags_;
    std::array<std::uint8_t, 3> reserved_;
    /** entry 和 option 部分 */
    std::uint32_t entriesLength;
    std::vector<std::shared_ptr<SdEntry>> entries_;
    std::uint32_t optionsLength;
    std::vector<std::shared_ptr<SdOption>> options_;
};

}  // namespace netflow::net::someip


#endif //TINYNETFLOW_SOMEIPSDMESSAGE_H
