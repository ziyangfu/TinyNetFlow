//
// Created by fzy on 23-12-5.
//

#ifndef OSADAPTOR_IO_IPC__SHM_IDENTIFIER_INFO_H
#define OSADAPTOR_IO_IPC__SHM_IDENTIFIER_INFO_H

#include <sys/types.h>
#include "IO/ipc/IpcMediaInfo.h"
#include "IO/ipc/shm/ShmConstant.h"

namespace osadaptor::ipc {
namespace shm {

/*!
 * \brief
 * */
struct ShmIdentifierInfo {
    UnixDomainPath path_{kIpcIndexDomainPortMin, kIpcIndexDomainPortMin};
    std::uint32_t size_ = kDefaultIpcFileSize;
    std::uint8_t* addr_{nullptr};
    pid_t pid_{-1};
    std::int32_t index_{-1};
};


}  // namespace shm
}  // namespace osadaptor::ipc


#endif //OSADAPTOR_IO_IPC__SHM_IDENTIFIER_INFO_H
