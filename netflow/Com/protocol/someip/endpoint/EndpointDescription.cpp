//
// Created by fzy on 23-9-20.
//

#include "EndpointDescription.h"

using namespace netflow::net::someip;

EndpointDescription::EndpointDescription(const std::string &address, uint16_t port, bool isUseTcp)
    : addr_(address),
      port_(port),
      remotePort_(0),  /** FIXME: remote Port 初始值应该是什么？ */
      isReliable_(isUseTcp)
{

}

EndpointDescription::~EndpointDescription() {
}

const std::string &EndpointDescription::getAddress() const {
    return addr_;
}

void EndpointDescription::setAddress(const std::string &address) {
    addr_ = address;
}

uint16_t EndpointDescription::getPort() const {
    return port_;
}

void EndpointDescription::setPort(uint16_t port) {
    port_ = port;
}

uint16_t EndpointDescription::getRemotePort() const {
    return remotePort_;
}

void EndpointDescription::setRemotePort(uint16_t port) {
    remotePort_ = port;
}

bool EndpointDescription::isReliable() const {
    return isReliable_;
}

void EndpointDescription::setReliable(bool isUseTcp) {
    isReliable_ = isUseTcp;
}