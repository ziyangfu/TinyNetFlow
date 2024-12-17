
/*!
 * 演示ShmEndpoint的读写功能
 * */

#include <iostream>
#include <thread>
#include <cstring>
#include "IO/ipc/shm/ShmWriter.h"
#include "IO/ipc/shm/ShmReader.h"

void writerThread(osadaptor::ipc::ShmWriter& writer) {
    writer.open();
    writer.start();

    char buffer[] = "Hello, Shared Memory!";
    writer.writeData(buffer, strlen(buffer) + 1);

    writer.stop();
    writer.close();
}

void readerThread(osadaptor::ipc::ShmReader& reader) {
    reader.open();
    reader.start();

    char buffer[100];
    reader.readData(buffer, sizeof(buffer));
    std::cout << "Read from shared memory: " << buffer << std::endl;

    reader.stop();
    reader.close();
}

int main() {
    std::string sharedMemoryPath = "/tmp/shared_memory";

    osadaptor::ipc::ShmWriter writer(sharedMemoryPath);
    osadaptor::ipc::ShmReader reader(sharedMemoryPath);

    std::thread writerThreadObj(writerThread, std::ref(writer));
    std::thread readerThreadObj(readerThread, std::ref(reader));

    writerThreadObj.join();
    readerThreadObj.join();

    return 0;
}

