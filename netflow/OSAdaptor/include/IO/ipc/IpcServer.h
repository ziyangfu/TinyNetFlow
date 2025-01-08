//
// Created by fzy on 2025/1/8.
//

#ifndef TINYNETFLOW_IPCSERVER_H
#define TINYNETFLOW_IPCSERVER_H
#include "IO/ipc/IpcMediaInfo.h"
namespace osadaptor::ipc {

class IpcServer {
public:
    IpcServer(IpcMediaInfo info);
    ~IpcServer();
    void start();
    void stop();
private:
    IpcMediaInfo info_;   /** uds or shm */
};

} // namespace osadaptor::ipc

#endif //TINYNETFLOW_IPCSERVER_H
