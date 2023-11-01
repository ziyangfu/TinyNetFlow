//
// Created by fzy on 23-9-12.
//

#ifndef TINYNETFLOW_SDOPTION_H
#define TINYNETFLOW_SDOPTION_H

#include <cstdint>
#include "netflow/someip/interface/EnumType.h"

namespace netflow::net::someip {

class SdOption {
public:
    SdOption(uint16_t len)
        : length(len),
          type_(SomeIpSdOptionType::UNKNOWN)
    {}
    virtual ~SdOption() = default;

    uint16_t getLength() const;
    SomeIpSdOptionType getSdOptionType() const;
protected:
    uint16_t length;
    SomeIpSdOptionType type_;
    std::uint8_t reserved_{0x00U};
};

}  // namespace netflow::net::someip



#endif //TINYNETFLOW_SDOPTION_H
