//
// Created by fzy on 23-5-26.
//

#ifndef TINYNETFLOW_TIMESTAMP_H
#define TINYNETFLOW_TIMESTAMP_H

#include <chrono>
#include <ctime>

namespace netflow::base {
/*!
 * \brief 封装 chrono， C++11 chrono缺失部分使用ctime补齐，预留C++20接口 */
class Timestamp {
public:
    Timestamp();
    ~Timestamp();
    std::chrono::time_point now();
private:
    std::chrono::time_point currentTimestamp;

};

}  // namespace netflow::base



#endif //TINYNETFLOW_TIMESTAMP_H
