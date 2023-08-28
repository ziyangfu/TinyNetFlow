//
// Created by fzy on 23-8-25.
//

#ifndef TINYNETFLOW_SOMEIPRUNTIME_H
#define TINYNETFLOW_SOMEIPRUNTIME_H

#include <map>
#include <mutex>
#include <memory>
#include <string>

#include "src/someip/Application.h"
#include "src/someip/SomeIpMessage.h"
#include "src/someip//SomeIpPayload.h"

namespace netflow::net::someip {

/** 所有apps均由runtime创建， 单例模式 */
class SomeIpRuntime {
public:
    static std::shared_ptr<SomeIpRuntime> get();
    /** runtime 属性，只有一个 */
    static std::string getProperty(const std::string& name);
    static void setProperty(const std::string& name, const std::string& value);


    std::shared_ptr<Application> createApplication(const std::string& name);
    std::shared_ptr<Application> createApplication(const std::string& name, const std::string& path);
    std::shared_ptr<Application>& getApplication(const std::string& name);
    void removeApplication(const std::string& name);


    /** 应该由应用负责 */
    std::shared_ptr<SomeIpMessage> createMessage();
    std::shared_ptr<SomeIpPayload> createPayload();
    void createNotification();


private:
    SomeIpRuntime();
    ~SomeIpRuntime();
private:
    static std::map<std::string, std::string> properties;
    static std::map<std::string, std::weak_ptr<Application>> apps_;   /** 一系列应用 */
    mutable std::mutex applicationsMutex_;

    static std::shared_ptr<SomeIpRuntime> runtime_;
    static SomeIpRuntime* run;
};


}  // namespace netflow::net::someip



#endif //TINYNETFLOW_SOMEIPRUNTIME_H
