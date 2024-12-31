
#include "../../../../../netflow/OSAdaptor/include/IO/ipc/internal/IpcProtocolHeader.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <cassert>

void printHexPayload(const std::string& payload) {
    std::stringstream ss;
    ss << "payload: ";
    for (unsigned char c : payload) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c) << " ";
    }
    std::cout << ss.str() << std::endl;
}
// 测试序列化和反序列化功能
void testSerializationDeserialization() {
    osadaptor::ipc::internal::IpcProtocolHeader header;
    header.length_ = 1024;
    header.magic_ = 0x7758;
    header.version_ = 0x01;
    header.type_ = osadaptor::ipc::internal::IpcProtocolType::SyncEvent;

    // 序列化
    std::string serialized = osadaptor::ipc::internal::IpcProtocolHeader::serializer(header);
    printHexPayload(serialized);

    // 反序列化
    osadaptor::ipc::internal::IpcProtocolHeader deserializedHeader =
            osadaptor::ipc::internal::IpcProtocolHeader::deserializer(serialized);

    // 验证反序列化后的值是否正确
    assert(deserializedHeader.length_ == header.length_);
    assert(deserializedHeader.magic_ == header.magic_);
    assert(deserializedHeader.version_ == header.version_);
    assert(deserializedHeader.type_ == header.type_);

    std::cout << "Test passed!" << std::endl;
}

int main() {
    testSerializationDeserialization();
    return 0;
}