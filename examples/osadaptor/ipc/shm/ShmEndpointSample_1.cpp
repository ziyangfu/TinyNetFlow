
/*!
 * 演示ShmEndpoint的读写功能
 * */

#include <iostream>
#include <thread>
#include <cstring>
#include "IO/ipc/shm/ShmEndpoint.h"

using namespace osadaptor::ipc;
class ShmEndpointSample {
public:
    ShmEndpointSample(){

    }
    ~ShmEndpointSample() {
        destroy();
    }
    void connect();

    void readData();
    void writeData();
private:
    void destroy();
private:
    ShmEndpoint endpoint_;
};

int main() {
    std::string sharedMemoryPath = "/tmp/shared_memory";
    ShmEndpointSample sample;
    sample.connect();
    sample.writeData();
    sample.readData();

    return 0;
}

