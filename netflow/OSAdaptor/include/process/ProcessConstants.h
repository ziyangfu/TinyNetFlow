/*!
 * \brief process 常量的定义
 * */

#ifndef OSADAPTOR_PROCESS_PROCESS_CONSTANTS_H
#define OSADAPTOR_PROCESS_PROCESS_CONSTANTS_H

#include <string>

namespace osadaptor::process {
/*!
 * \brief 默认的最大隔离cpu数量, 且设置CPU核心为 CPU0，CPU1
 * */
constexpr int kDefaultIsolatedCpuCountMax = 2;

/*!
 * \brief CPU隔离核心的设置路径
 * */
const std::string kIsolatedCpuSetDir = "/sys/devices/system/cpu/isolated";

}  // namespace osadaptor::process

#endif //OSADAPTOR_PROCESS_PROCESS_CONSTANTS_H
