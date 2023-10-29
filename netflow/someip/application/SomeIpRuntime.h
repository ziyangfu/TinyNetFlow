//
// Created by fzy on 23-8-25.
//

#ifndef TINYNETFLOW_SOMEIPRUNTIME_H
#define TINYNETFLOW_SOMEIPRUNTIME_H

#include <map>
#include <mutex>
#include <memory>
#include <string>

#include "netflow/someip/interface/Runtime.h"

#include "netflow/someip/application/ApplicationImpl.h"
#include "netflow/someip/message/SomeIpMessageImpl.h"
#include "netflow/someip/message/SomeIpPayload.h"

namespace netflow::net::someip {

/** 所有apps均由runtime创建， 并进行管理 */
class SomeIpRuntime : public Runtime {
public:
    SomeIpRuntime();
    ~SomeIpRuntime();

    static std::shared_ptr<Runtime> get();
    /** runtime 属性，只有一个 */
    static std::string getProperty(const std::string& name);
    static void setProperty(const std::string& name, const std::string& value);


    std::shared_ptr<Application> createApplication(const std::string &name = "") override;

    std::shared_ptr<Message> createMessage()  override;
    std::shared_ptr<Message> createRequest()  override;
    std::shared_ptr<Message> createNotification()  override;

    std::shared_ptr<Message> createResponse(const std::shared_ptr<Message> &request)  override;

    std::shared_ptr<Payload> createPayload()  override;

    void removeApplication(const std::string& name);
    std::shared_ptr<Application> getApplication(const std::string &name) const;
private:
    std::shared_ptr<Application> createApplication(const std::string &name, const std::string &path);

private:
    static std::map<std::string, std::string> properties;
    std::map<std::string, std::weak_ptr<Application>> apps_;   /** 一系列应用 */
    mutable std::mutex applicationsMutex_;
};


}  // namespace netflow::net::someip



#endif //TINYNETFLOW_SOMEIPRUNTIME_H
