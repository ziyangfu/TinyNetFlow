//
// Created by fzy on 23-9-12.
//

#ifndef TINYNETFLOW_SDOPTION_H
#define TINYNETFLOW_SDOPTION_H

#include <cstdint>
#include "src/someip/SomeIpProtocol.h"

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
};

}  // namespace netflow::net::someip



#endif //TINYNETFLOW_SDOPTION_H
