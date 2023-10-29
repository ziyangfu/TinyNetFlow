//
// Created by fzy on 23-8-28.
//

#include "SdMessage.h"

#include "netflow/someip/interface/Constants.h"


using namespace netflow::net::someip;

SdMessage::SdMessage()
{
    header_.serviceId_      = kSdServiceId;
    header_.methodId_       = kSdMethodId;
    header_.messageType_    = SomeIpMessageType::MT_NOTIFICATION;
    header_.returnCode_     = SomeIpReturnCode::E_OK;
}
