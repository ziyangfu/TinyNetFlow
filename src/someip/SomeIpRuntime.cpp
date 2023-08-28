//
// Created by fzy on 23-8-25.
//

#include "SomeIpRuntime.h"



using namespace netflow::net::someip;

std::shared_ptr<SomeIpRuntime> SomeIpRuntime::runtime_ = nullptr;
SomeIpRuntime* SomeIpRuntime::run = nullptr;

SomeIpRuntime::SomeIpRuntime() {

}

SomeIpRuntime::~SomeIpRuntime(){

}

/*!
 * \brief 创建一个runtime
 * \static */
std::shared_ptr<SomeIpRuntime> SomeIpRuntime::get() {
    if (runtime_ == nullptr) {
        run = new SomeIpRuntime();
        run = runtime_.get();
    }


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

}

std::shared_ptr<Application> SomeIpRuntime::createApplication(const std::string &name, const std::string &path) {

}

std::shared_ptr<Application> &SomeIpRuntime::getApplication(const std::string &name) {

}

void SomeIpRuntime::removeApplication(const std::string &name) {

}

