//
// Created by fzy on 2024/12/17.
//


#ifndef OSADAPTOR_IO_IPC_SHM__SHM_ENDPOINT_H
#define OSADAPTOR_IO_IPC_SHM__SHM_ENDPOINT_H

#include "IO/ipc/shm/ShmReader.h"
#include "IO/ipc/shm/ShmWriter.h"

namespace osadaptor {
namespace ipc {

class ShmEndpoint {
public:
    ShmEndpoint(const std::string& shmPath, const std::string& shmName, std::uint32_t shmSize);
    void start();
    void stop();
    void readData(void* buffer, size_t bufferSize);
    void writeData(const void* buffer, size_t bufferSize);
private:
    ShmReader reader_;
    ShmWriter writer_;
};

} // ipc
} // osadaptor

#endif //OSADAPTOR_IO_IPC_SHM__SHM_ENDPOINT_H
