//
// Created by fzy on 2025/1/8.
//

/*!
说明
命名规范:

使用 k 前缀表示常量。
使用 EventType 枚举类来表示事件类型。
使用 Encode 方法来编码数据。
使用 ToString 方法来生成字符串表示。
Header 类:

使用 std::vector<uint8_t> 来存储头部数据。
提供 Length, Magic, Version, MsgType 方法来访问头部字段。
提供 Encode 方法来编码头部数据。
提供 ToString 方法来生成头部的字符串表示。
FallbackDataEvent 类:

使用 std::vector<uint8_t> 来存储事件数据。
提供 Encode 方法来编码事件数据。
辅助函数:

EventTypeToString 函数将 EventType 转换为字符串。
CreateInvalidVersionError 和 CreateInvalidMsgTypeError 函数用于创建错误信息。
CheckEventValid 函数检查头部的有效性。
InitializePollingEvents 函数初始化 pollingEventWithVersion 数组。
main 函数:

示例使用 Header 和 FallbackDataEvent 类。
注意事项
内存管理: 使用 std::vector<uint8_t> 来管理动态数组，确保内存安全。
错误处理: 使用 std::runtime_error 来处理错误情况。
字节序转换: 使用 htobe32 和 be32toh 等函数进行字节序转换。


 *
 * */

#ifndef TINYNETFLOW_PROTOCOLEVENT_H
#define TINYNETFLOW_PROTOCOLEVENT_H
#include <cstdint>

#include <cstdint>
#include <vector>
#include <string>
#include <sstream>
#include <stdexcept>
#include <endian.h>
#include <cstring>

namespace shmipc {

constexpr uint16_t magicNumber = 0x1234;
constexpr size_t headerSize = 8;
constexpr size_t maxSupportProtoVersion = 10;

enum class EventType : uint8_t {
    kShareMemoryByFilePath      = 0,
    kPolling                    = 1,
    kStreamClose                = 2,
    kFallbackData               = 3,
    kExchangeProtoVersion       = 4,
    kShareMemoryByMemfd         = 5,
    kAckShareMemory             = 6,
    kAckReadyRecvFD             = 7,
    kHotRestart                 = 8,
    kHotRestartAck              = 9,

    kMinEventType               = kShareMemoryByFilePath,
    kMaxEventType               = kHotRestartAck
};

std::vector<uint8_t> pollingEventWithVersion[maxSupportProtoVersion + 1];

class Header {
public:
    Header() : data_(headerSize, 0) {}

    uint32_t Length() const { return be32toh(*reinterpret_cast<const uint32_t*>(data_.data())); }
    uint16_t Magic() const { return be16toh(*reinterpret_cast<const uint16_t*>(data_.data() + 4)); }
    uint8_t Version() const { return data_[6]; }
    EventType MsgType() const { return static_cast<EventType>(data_[7]); }

    std::string ToString() const {
        std::ostringstream oss;
        oss << "Length:" << Length() << " Magic:" << Magic() << " Version:" << static_cast<int>(Version())
            << " Type:" << EventTypeToString(MsgType());
        return oss.str();
    }

    void Encode(uint32_t length, uint8_t version, EventType msgType) {
        uint32_t len = htobe32(length);
        uint16_t mag = htobe16(magicNumber);
        std::memcpy(data_.data(), &len, 4);
        std::memcpy(data_.data() + 4, &mag, 2);
        data_[6] = version;
        data_[7] = static_cast<uint8_t>(msgType);
    }
    static std::string EventTypeToString(EventType type) {
        switch (type) {
            case EventType::kShareMemoryByFilePath:
                return "ShareMemoryByFilePath";
            case EventType::kPolling:
                return "Polling";
            case EventType::kStreamClose:
                return "StreamClose";
            case EventType::kFallbackData:
                return "FallbackData";
            case EventType::kExchangeProtoVersion:
                return "ExchangeProtoVersion";
            case EventType::kShareMemoryByMemfd:
                return "ShareMemoryByMemfd";
            case EventType::kAckShareMemory:
                return "AckShareMemory";
            case EventType::kAckReadyRecvFD:
                return "AckReadyRecvFD";
            case EventType::kHotRestart:
                return "HotRestart";
            case EventType::kHotRestartAck:
                return "HotRestartAck";
            default:
                return "<UNSET>" + std::to_string(static_cast<int>(type));
        }
    }

    std::vector<uint8_t> data() const { return data_; }

private:
    std::vector<uint8_t> data_;
};

class FallbackDataEvent {
public:
    static constexpr size_t kSize = headerSize + 8;

    FallbackDataEvent() : data_(kSize, 0) {}

    void Encode(uint32_t length, uint8_t version, uint32_t seqID, uint32_t status) {
        uint32_t len = htobe32(length);
        uint16_t mag = htobe16(magicNumber);
        uint32_t seq = htobe32(seqID);
        uint32_t stat = htobe32(status);
        std::memcpy(data_.data(), &len, 4);
        std::memcpy(data_.data() + 4, &mag, 2);
        data_[6] = version;
        data_[7] = static_cast<uint8_t>(EventType::kFallbackData);
        std::memcpy(data_.data() + 8, &seq, 4);
        std::memcpy(data_.data() + 12, &stat, 4);
    }

private:
    std::vector<uint8_t> data_;
};



std::runtime_error CreateInvalidVersionError(uint16_t magic, uint8_t version) {
    std::ostringstream oss;
    oss << "shmipc: Invalid magic or version " << magic << ", " << static_cast<int>(version);
    return std::runtime_error(oss.str());
}

std::runtime_error CreateInvalidMsgTypeError(const Header& hdr) {
    std::ostringstream oss;
    oss << "shmipc, invalid protocol header: " << hdr.ToString();
    return std::runtime_error(oss.str());
}

std::runtime_error CheckEventValid(const Header& hdr) {
    if (hdr.Magic() != magicNumber || hdr.Version() == 0) {
        return CreateInvalidVersionError(hdr.Magic(), hdr.Version());
    }
    EventType mt = hdr.MsgType();
    if (mt < EventType::kMinEventType || mt > EventType::kMaxEventType) {
        return CreateInvalidMsgTypeError(hdr);
    }
    return std::runtime_error("");
}

void InitializePollingEvents() {
    for (int i = 0; i < maxSupportProtoVersion + 1; ++i) {
        pollingEventWithVersion[i] = std::vector<uint8_t>(headerSize, 0);
        Header header;
        header.Encode(headerSize, static_cast<uint8_t>(i), EventType::kPolling);
        pollingEventWithVersion[i] = header.data();
    }
}

}  // namespace shmipc


//int main() {
//    shmipc::InitializePollingEvents();
//
//    shmipc::Header header;
//    header.Encode(shmipc::headerSize, 1, shmipc::EventType::kPolling);
//    std::cout << header.ToString() << std::endl;
//
//    shmipc::FallbackDataEvent fallbackEvent;
//    fallbackEvent.Encode(shmipc::FallbackDataEvent::kSize, 1, 12345, 0);
//    // Example usage of fallbackEvent
//
//    return 0;
//}

#endif //TINYNETFLOW_PROTOCOLEVENT_H
