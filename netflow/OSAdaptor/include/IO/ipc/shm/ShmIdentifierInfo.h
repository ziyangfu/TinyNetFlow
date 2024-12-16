//
// Created by fzy on 23-12-5.
//

#ifndef OSADAPTOR_IO_IPC_SHM_IPCTYPEINFO_H
#define OSADAPTOR_IO_IPC_SHM_IPCTYPEINFO_H

#include <sys/types.h>
#include "IO/ipc/IpcMediaAddr.h"
#include "IO/ipc/shm/ShmConstant.h"

namespace osadaptor::ipc {
namespace shm {

/*!
 * \brief
 * */
struct ShmIdentifierInfo {
    UnixDomainPath path_{10,10};
    std::uint32_t size_ = kDefaultShmFileSize;
    pid_t pid_{-1};
    std::int32_t index_{-1};
};


}  // namespace shm
}  // namespace osadaptor::ipc


#endif //OSADAPTOR_IO_IPC_SHM_IPCTYPEINFO_H
