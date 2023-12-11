//
// Created by fzy on 23-8-25.
//

#include "SomeIpHeader.h"

using namespace netflow::net::someip;

SomeIpHeader::SomeIpHeader()
    : serviceId_(0x0),
      methodId_(0x0),
      length_(0x00),
      clientId_(0x0),
      sessionId_(0x0),
      protocolVersion_(0x1),
      interfaceVersion_(0x0),
      messageType_(SomeIpMessageType::MT_UNKNOWN),
      returnCode_(SomeIpReturnCode::E_UNKNOWN),
      instanceId_(0x0),
      messageId_(0x00),
      requestId_(0x00)
{

}

SomeIpHeader::SomeIpHeader(const SomeIpHeader &header)
    : serviceId_(header.sessionId_),
      methodId_(header.methodId_),
      length_(header.length_),
      clientId_(header.clientId_),
      sessionId_(header.sessionId_),
      protocolVersion_(header.protocolVersion_),
      interfaceVersion_(header.interfaceVersion_),
      messageType_(header.messageType_),
      returnCode_(header.returnCode_),
      instanceId_(header.instanceId_),
      messageId_(header.messageId_),
      requestId_(header.requestId_)
{

}

SomeIpHeader::~SomeIpHeader() {

}