//
// Created by fzy on 23-9-12.
//

#ifndef TINYNETFLOW_SDOPTIONLOADBALANCING_H
#define TINYNETFLOW_SDOPTIONLOADBALANCING_H


#include "SdOption.h"
#include "netflow/someip/interface/TypeDefine.h"

namespace netflow::net::someip {
/** 负载均衡部分， 暂时不实现 */
class SdOptionLoadBalancing : public SdOption {
public:
    explicit SdOptionLoadBalancing(Priority priority, Weight weight);
    ~SdOptionLoadBalancing() = default;
private:
    Priority    priority_;
    Weight      weight_;
};

}  // namespace netflow::net::someip


#endif //TINYNETFLOW_SDOPTIONLOADBALANCING_H
