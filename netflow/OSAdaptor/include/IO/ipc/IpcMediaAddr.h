//
// Created by fzy on 23-12-21.
//

#ifndef TINYNETFLOW_IPCMEDIAADDR_H
#define TINYNETFLOW_IPCMEDIAADDR_H

/** 定义 uds 与 shared memory 的统一地址 ， 采用 domain与port来描述 */

namespace netflow::osadaptor::ipc {
struct UnixDomainPath {
    int domain;
    int port;
};

/*!
 * \brief IPC的两种方式
 * */
enum class IpcType : std::uint8_t {
    kUds,
    kShm
};
} // namespace netflow::osadaptor::ipc


#endif //TINYNETFLOW_IPCMEDIAADDR_H
