//
// Created by fzy on 23-9-12.
//

#ifndef TINYNETFLOW_SDOPTIONIPV4_H
#define TINYNETFLOW_SDOPTIONIPV4_H

#include "SdOption.h"
#include <string>

namespace netflow::net::someip {
/*!
 * \brief  ipv4 option 与 ipv4 multicast option */
class SdOptionIpv4 : public SdOption {
public:
    explicit SdOptionIpv4(bool isMulticast, std::string& addr, uint16_t port);
    ~SdOptionIpv4() = default;

    const std::string getAddr() const;
    void setAddr(std::string& addr);

    uint16_t getPort() const;
    void setPort(uint16_t port);

    bool isUsingUdp();
    bool settingUseUdp(bool on);

    bool isMulticast() const;
private:
    /** ipv4 地址 */
    std::string addr_;
    uint16_t port_;
    bool isUdp_;
};


}  // namespace netflow::net::someip



#endif //TINYNETFLOW_SDOPTIONIPV4_H
