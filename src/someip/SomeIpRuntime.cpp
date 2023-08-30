//
// Created by fzy on 23-8-25.
//

#include "SomeIpRuntime.h"



using namespace netflow::net::someip;

std::map<std::string, std::string> SomeIpRuntime::properties;  /** 静态成员变量 */

SomeIpRuntime::SomeIpRuntime() {

}

SomeIpRuntime::~SomeIpRuntime(){

}

std::shared_ptr<SomeIpRuntime> SomeIpRuntime::get() {
    static std::shared_ptr<SomeIpRuntime> runtime = std::make_unique<SomeIpRuntime>();
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
    return createApplication(name, "");
}

std::shared_ptr<Application> SomeIpRuntime::createApplication(const std::string &name, const std::string &path) {
    static uint32_t id = 0;
    std::string its_name = name;

    std::lock_guard<std::mutex> app_lock(applicationsMutex_);
    auto found_application_itr = apps_.find(name);
    /** 找到了，新开一个副本，新命名，保证不重名 */
    if (found_application_itr != apps_.end()) {
        its_name += "_" +std::to_string(id++);
    }
    auto application = std::make_shared<Application>(); /** FIXME */
    apps_[its_name] = application;
    return application;
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

