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
    virtual ~Runtime() = default;

    virtual std::shared_ptr<Application> createApplication(const std::string& name = "") const = 0;

    virtual std::shared_ptr<Message> createMessage() const = 0;
    virtual std::shared_ptr<Message> createRequest() const = 0;
    virtual std::shared_ptr<Message> createResponse(const std::shared_ptr<Message>& request) const = 0;
    virtual std::shared_ptr<Message> createNotification() const = 0;

    virtual std::shared_ptr<Payload> createPayload() const = 0;
};
}  // namespace netflow::net::someip

#endif //TINYNETFLOW_RUNTIME_H
