//
// Created by fzy on 23-8-28.
//

#ifndef TINYNETFLOW_SOMEIPPAYLOAD_H
#define TINYNETFLOW_SOMEIPPAYLOAD_H

#include <string>
#include <vector>
#include "netflow/Com/someip/interface/Payload.h"

/** 表示一条 SOME/IP 消息的负载 */
namespace netflow::net::someip {

class SomeIpPayload final : public Payload{
public:
    SomeIpPayload();
    ~SomeIpPayload();

    void setPayload(const std::vector<uint8_t> &data) override;
    void setPayload(const std::string &data) override;
    void setPayload(const uint8_t *data, int length) override;
    void setCapacity(size_t capacity) override;

    const uint8_t * getPayload() override;
    std::string & getPayload(bool isString) override;
    void getPayload(std::vector<uint8_t> &data) override;

    size_t getLength() override;

    bool operator==(const Payload& other) override;

private:
    std::vector<uint8_t> data_;
};

}  // namespace netflow::net::someip



#endif //TINYNETFLOW_SOMEIPPAYLOAD_H
