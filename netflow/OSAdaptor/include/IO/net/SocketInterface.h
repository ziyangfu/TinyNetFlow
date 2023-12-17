//
// Created by fzy on 23-12-15.
//

#ifndef TINYNETFLOW_SOCKETINTERFACE_H
#define TINYNETFLOW_SOCKETINTERFACE_H

#include <cstdlib>
#include <cstdint>

namespace netflow::osadaptor {
int createSocket(std::int32_t family, std::int32_t type, std::int32_t protocol) noexcept;

void close(int fd) noexcept;

void bind();

void setNoBlocking();

void getSockName();

void getPeerSocket();

void setSocketOption(/** fd, level, optionName ...*/);

void getSocketOption();




} // namespace netflow::osadaptor


#endif //TINYNETFLOW_SOCKETINTERFACE_H
