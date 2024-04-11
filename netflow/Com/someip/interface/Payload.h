//
// Created by fzy on 23-10-26.
//

#ifndef TINYNETFLOW_PAYLOAD_H
#define TINYNETFLOW_PAYLOAD_H

#include <vector>
#include <cstdint>
#include <string>

/** Payload 接口 */
namespace netflow::net::someip {

class Payload {
public:
    virtual ~Payload() {}
    virtual bool operator==(const Payload& other) = 0;

    virtual void setPayload(const std::vector<uint8_t>& data) = 0;
    virtual void setPayload(const std::string& data) = 0;
    virtual void setPayload(const uint8_t* data, int length) = 0;
    virtual void setCapacity(size_t capacity) = 0;

    virtual void getPayload(std::vector<uint8_t>& data) = 0;
    virtual std::string& getPayload(bool isString) = 0;
    virtual const uint8_t* getPayload() = 0;
    virtual size_t getLength() = 0;
};

}  // namespace netflow::net::someip
#endif //TINYNETFLOW_PAYLOAD_H
