//
// Created by fzy on 23-12-4.
//

#ifndef TINYNETFLOW_OSADAPTOR_PREDEFINE_SHM_H
#define TINYNETFLOW_OSADAPTOR_PREDEFINE_SHM_H

#include <string>
#include "IO/ipc/IpcMediaAddr.h"

namespace netflow::osadaptor::ipc {

namespace shm {

auto kDefaultIpcType {IpcType::kUds};
/*!
 * \brief mmap共享内存预定义参数
 * */
const std::string kDefaultSharedMemoryPath {"/tmp/netflow_shm_domain_10_port_10"};

constexpr std::uint32_t kDefaultFileSize { 64 * 1024 }; /** 编译器计算，默认64K bytes */


}  // namespace shm

} // namespace netflow::osadaptor::ipc


#endif //TINYNETFLOW_OSADAPTOR_PREDEFINE_SHM_H
