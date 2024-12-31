//
// Created by fzy on 2024/12/17.
//

#include "IO/ipc/shm/ShmEndpoint.h"

namespace osadaptor {
namespace ipc {

/*!
 * \brief
 * \details
 *      writer 创建writer2Reader.shm，映射共享内存
 *      reader 映射共享内存，打开reader2Writer.shm（谁来创建？对端endpoint创建，我方怎么知道domain， port）
 *              同步机制采用uds或tcp，首先建立连接，然后发送元数据，之后写入数据后通知对端
 *
 **/
ShmEndpoint::ShmEndpoint() {

}

ShmEndpoint::ShmEndpoint(const std::string &shmPath, const std::string &shmName, std::uint32_t shmSize) {

}

ShmEndpoint::~ShmEndpoint() {

}

void ShmEndpoint::readMessage(void *buffer, size_t bufferSize) {

}

void ShmEndpoint::writeMessage(const void *buffer, size_t bufferSize) {

}


} // ipc
} // osadaptor