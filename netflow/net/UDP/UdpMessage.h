//
// Created by fzy on 23-8-24.
//

#ifndef TINYNETFLOW_UDPMESSAGE_H
#define TINYNETFLOW_UDPMESSAGE_H

namespace netflow::net {

inline bool sendMessage(int fd, const struct sockaddr* addr, const std::string& message) {

}

inline bool sendMessage(int fd, const struct sockaddr* addr, const char* data, size_t length) {
    if (length == 0) {
        return true;
    }
    int sendNum = ::sendto(fd, data, length, 0, addr, sizeof(*addr));
    if (sendNum != length) {
        return false;
    }
    return true;
}
}  // namespace netflow::net


#endif //TINYNETFLOW_UDPMESSAGE_H
