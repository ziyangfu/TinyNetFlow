//
// Created by fzy on 23-8-25.
//

#include "SomeIpRuntime.h"
#include "netflow/someip/interface/Constants.h"

using namespace netflow::net::someip;

std::map<std::string, std::string> SomeIpRuntime::properties;  /** 静态成员变量 */

SomeIpRuntime::SomeIpRuntime() {

}

SomeIpRuntime::~SomeIpRuntime(){

}

std::shared_ptr<Runtime> SomeIpRuntime::get() {
    static std::shared_ptr<Runtime> runtime = std::make_unique<SomeIpRuntime>();
    return runtime;
}

/*!
 * \static */
std::string SomeIpRuntime::getProperty(const std::string &name) {

}
/*!
 * \static */
void SomeIpRuntime::setProperty(const std::string &name, const std::string &value) {

}

std::shared_ptr<Application> SomeIpRuntime::createApplication(const std::string &name) {
    return std::make_shared<ApplicationImpl>();
}

std::shared_ptr<Application> SomeIpRuntime::createApplication(const std::string &name, const std::string &path) {
    /**
    static uint32_t id = 0;
    std::string its_name = name;

    std::lock_guard<std::mutex> app_lock(applicationsMutex_);
    auto found_application_itr = apps_.find(name);
    // 找到了，新开一个副本，新命名，保证不重名
    if (found_application_itr != apps_.end()) {
        its_name += "_" +std::to_string(id++);
    }
    apps_[its_name] = application;
    return application;
    */
}

std::shared_ptr<Application> SomeIpRuntime::getApplication(const std::string &name) const {
    std::lock_guard<std::mutex> lock(applicationsMutex_);
    auto itr = apps_.find(name);
    if (itr != apps_.end()) {
        return itr->second.lock();
    }
    return nullptr;
}

void SomeIpRuntime::removeApplication(const std::string &name) {
    std::lock_guard<std::mutex> lock(applicationsMutex_);
    auto itr = apps_.find(name);
    if (itr != apps_.end()) {
        apps_.erase(name);
    }
}

std::shared_ptr<Message> SomeIpRuntime::createMessage(){
    std::shared_ptr<Message> message = std::make_shared<SomeIpMessageImpl>();
    message->setProtocolVersion(kSomeIpProtocolVersion);
    message->setReturnCode(SomeIpReturnCode::E_OK);
    return message;
}

std::shared_ptr<Message> SomeIpRuntime::createRequest(){
    std::shared_ptr<Message> request = std::make_shared<SomeIpMessageImpl>();
    request->setProtocolVersion(kSomeIpProtocolVersion);
    request->setMessageType(SomeIpMessageType::MT_REQUEST);
    request->setReturnCode(SomeIpReturnCode::E_OK);
    return request;
}

std::shared_ptr<Message> SomeIpRuntime::createResponse(const std::shared_ptr<Message> &request){
    std::shared_ptr<Message> response = std::make_shared<SomeIpMessageImpl>();
    response->setServiceId(request->getServiceId());
    response->setMethodId(request->getMethodId());
    response->setClientId(request->getClientId());
    response->setSessionId(request->getSessionId());

    response->setInterfaceVersion(request->getInterfaceVersion());
    response->setMessageType(request->getMessageType());
    response->setReturnCode(request->getReturnCode());

    response->setInstanceId(request->getInstanceId());
    return response;
}

std::shared_ptr<Message> SomeIpRuntime::createNotification(){
    std::shared_ptr<Message> notification = std::make_shared<SomeIpMessageImpl>();

    notification->setProtocolVersion(kSomeIpProtocolVersion);
    notification->setMessageType(SomeIpMessageType::MT_NOTIFICATION);
    notification->setReturnCode(SomeIpReturnCode::E_OK);
    return notification;
}

std::shared_ptr<Payload> SomeIpRuntime::createPayload(){
    return std::make_shared<SomeIpPayload>();
}

