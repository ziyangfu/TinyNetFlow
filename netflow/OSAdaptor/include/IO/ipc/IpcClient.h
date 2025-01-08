//
// Created by fzy on 2025/1/8.
//

#ifndef TINYNETFLOW_IPCCLIENT_H
#define TINYNETFLOW_IPCCLIENT_H

namespace osadaptor::ipc {

class IpcClient {
public:
    IpcClient(const std::string &path);
    ~IpcClient();

    void connect();
    void disconnect();

    void send(const std::string &msg);
    std::string recv();

private:
    int m_fd;
    std::string m_path;
};

}  // namespace osadaptor::ipc

#endif //TINYNETFLOW_IPCCLIENT_H
