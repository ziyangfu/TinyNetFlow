//
// Created by fzy on 23-9-12.
//

#ifndef TINYNETFLOW_SDOPTIONIPV6_H
#define TINYNETFLOW_SDOPTIONIPV6_H

#include "SdOption.h"
#include <string>


namespace netflow::net::someip {
/*!
 * \brief  ipv6 option 与 ipv6 multicast option
 *         继承，但不是多态 */
class SdOptionIpv6 : public SdOption {
public:
    explicit SdOptionIpv6(bool isMulticast, std::string& addr6, uint16_t port);
    ~SdOptionIpv6()= default;

    const std::string getAddr6() const;
    void setAddr6(std::string& addr6);

    uint16_t getPort() const;
    void setPort(uint16_t port);

    bool isUsingUdp();
    bool settingUseUdp(bool on);

    bool isMulticast() const;

private:
    /** ipv6 地址 */
    std::string addr6_;
    uint16_t port_;
    bool isUdp_;
};

}  // namespace netflow::net::someip

#endif //TINYNETFLOW_SDOPTIONIPV6_H
