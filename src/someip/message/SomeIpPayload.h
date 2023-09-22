//
// Created by fzy on 23-8-28.
//

#ifndef TINYNETFLOW_SOMEIPPAYLOAD_H
#define TINYNETFLOW_SOMEIPPAYLOAD_H

#include <string>
#include <vector>

/** 表示一条 SOME/IP 消息的负载 */
namespace netflow::net::someip {

class SomeIpPayload {
public:
    SomeIpPayload();
    ~SomeIpPayload();

    void setData(const uint8_t* data, const uint32_t len);
    void setData(const std::vector<uint8_t>& data);
    void getData();

    bool operator==(const SomeIpPayload& other);
private:
    std::vector<uint8_t> data_;
    std::string message;  /** or */
};

}  // namespace netflow::net::someip



#endif //TINYNETFLOW_SOMEIPPAYLOAD_H
