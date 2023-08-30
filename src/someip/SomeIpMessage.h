//
// Created by fzy on 23-8-22.
//

#ifndef TINYNETFLOW_SOMEIPMESSAGE_H
#define TINYNETFLOW_SOMEIPMESSAGE_H

#include <stdint.h>
#include <vector>
#include <memory>

#include "src/someip/SomeIpHeader.h"
#include "src/someip//SomeIpPayload.h"


/** 表示一条SOMEIP消息 */
namespace netflow::net::someip {

class SomeIpMessage {
public:
    void setPayload();
    void getPayload();
    SomeIpHeader& getHeader() {return header_; }
private:
    /**
     * SomeIpHeader
     * Payload
     * */
     SomeIpHeader header_;
     std::shared_ptr<SomeIpPayload> payloadPtr;
};


} // namespace netflow::net::someip



#endif //TINYNETFLOW_SOMEIPMESSAGE_H
