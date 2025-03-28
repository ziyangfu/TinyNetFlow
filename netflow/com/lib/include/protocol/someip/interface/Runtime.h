//
// Created by fzy on 23-9-10.
//

#ifndef TINYNETFLOW_RUNTIME_H
#define TINYNETFLOW_RUNTIME_H

#include <memory>
#include <string>
namespace netflow::net::someip {

class Application;
class Message;
class Payload;

class Runtime {
public:
    Runtime() = default;
    virtual ~Runtime() {}
    /** 静态函数，在application/Runtime.cpp中实现 */
    static std::shared_ptr<Runtime> get();

    virtual std::shared_ptr<Application> createApplication(const std::string& name = "") = 0;

    virtual std::shared_ptr<Message> createMessage()  = 0;
    virtual std::shared_ptr<Message> createRequest()  = 0;
    virtual std::shared_ptr<Message> createResponse(const std::shared_ptr<Message>& request) = 0;
    virtual std::shared_ptr<Message> createNotification() = 0;

    virtual std::shared_ptr<Payload> createPayload() = 0;
};

}  // namespace netflow::net::someip

#endif //TINYNETFLOW_RUNTIME_H
