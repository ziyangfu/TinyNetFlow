
#include "src/someip/Application.h"

#include "src/someip/SomeIpMessage.h"
#include "src/someip/SomeIpProtocol.h"
#include "src/someip/constants.h"

using namespace netflow::net::someip;

std::shared_ptr<SomeIpMessage> Application::createMessage() {
    auto message = std::make_shared<SomeIpMessage>();
    auto& header = message->getHeader().getHeader();
    header.protocolVersion_ = SOMEIP_PROTOCOL_VERSION;  /** 放在header构造函数里？*/
    header.returnCode_ = SomeIpReturnCode::E_OK;
    header.interfaceVersion_ = 0;
    return message;
}

std::shared_ptr<SomeIpMessage> Application::createRequest() {

}

std::shared_ptr<SomeIpMessage> Application::createResponse() {

}

std::shared_ptr<SomeIpMessage> Application::createNotification() {


}

std::shared_ptr<SomeIpPayload> Application::createPayload() {

}


