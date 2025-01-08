//
// Created by fzy on 2025/1/8.
//

/*!

 * */

#ifndef TINYNETFLOW_PROTOCOLMANAGER_H
#define TINYNETFLOW_PROTOCOLMANAGER_H

#include <cstdint>
#include <vector>
#include <string>
#include <sstream>
#include <stdexcept>
#include <endian.h>
#include <cstring>
#include <memory>
#include <functional>
#include <atomic>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <algorithm>

namespace shmipc {

constexpr uint16_t kMagicNumber = 0x1234;
constexpr size_t kHeaderSize = 8;
constexpr size_t kMaxSupportProtoVersion = 10;
constexpr size_t kEpochIDLen = 8;
constexpr size_t kMemfdCount = 2;
constexpr size_t kMemfdDataLen = 4;

enum class EventType : uint8_t {
    kShareMemoryByFilePath = 0,
    kPolling = 1,
    kStreamClose = 2,
    kFallbackData = 3,
    kExchangeProtoVersion = 4,
    kShareMemoryByMemfd = 5,
    kAckShareMemory = 6,
    kAckReadyRecvFD = 7,
    kHotRestart = 8,
    kHotRestartAck = 9,

    kMinEventType = kShareMemoryByFilePath,
    kMaxEventType = kHotRestartAck
};

using ProtocolHandler = std::function<std::tuple<int, bool, std::runtime_error>(Session*, const Header&, const std::vector<uint8_t>&)>;

std::vector<ProtocolHandler> protocolHandlers = {
        nullptr,  // kShareMemoryByFilePath
        nullptr,  // kPolling
        nullptr,  // kStreamClose
        nullptr,  // kFallbackData
        nullptr,  // kExchangeProtoVersion
        nullptr,  // kShareMemoryByMemfd
        nullptr,  // kAckShareMemory
        nullptr,  // kAckReadyRecvFD
        nullptr,  // kHotRestart
        nullptr   // kHotRestartAck
};

bool protocol_trace_mode = false;

class Header {
public:
    Header() : data_(kHeaderSize, 0) {}

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
        uint16_t mag = htobe16(kMagicNumber);
        std::memcpy(data_.data(), &len, 4);
        std::memcpy(data_.data() + 4, &mag, 2);
        data_[6] = version;
        data_[7] = static_cast<uint8_t>(msgType);
    }

private:
    std::vector<uint8_t> data_;
};

class FallbackDataEvent {
public:
    static constexpr size_t kSize = kHeaderSize + 8;

    FallbackDataEvent() : data_(kSize, 0) {}

    void Encode(uint32_t length, uint8_t version, uint32_t seqID, uint32_t status) {
        uint32_t len = htobe32(length);
        uint16_t mag = htobe16(kMagicNumber);
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

std::string EventTypeToString(EventType type) {
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
    if (hdr.Magic() != kMagicNumber || hdr.Version() == 0) {
        return CreateInvalidVersionError(hdr.Magic(), hdr.Version());
    }
    EventType mt = hdr.MsgType();
    if (mt < EventType::kMinEventType || mt > EventType::kMaxEventType) {
        return CreateInvalidMsgTypeError(hdr);
    }
    return std::runtime_error("");
}

void InitializeProtocolHandlers() {
    protocolHandlers[static_cast<int>(EventType::kPolling)] = handlePolling;
    protocolHandlers[static_cast<int>(EventType::kStreamClose)] = handleStreamClose;
    protocolHandlers[static_cast<int>(EventType::kFallbackData)] = handleFallbackData;
    protocolHandlers[static_cast<int>(EventType::kHotRestart)] = handleHotRestart;
    protocolHandlers[static_cast<int>(EventType::kHotRestartAck)] = handleHotRestartAck;
}

void InitializeProtocolTraceMode() {
    if (!std::getenv("SHMIPC_PROTOCOL_TRACE")) {
        protocol_trace_mode = true;
    }
}

class ProtocolAdaptor {
public:
    explicit ProtocolAdaptor(Session* session) : session_(session) {}

    std::shared_ptr<ProtocolInitializer> GetProtocolInitializer() {
        if (session_->is_client) {
            return ClientGetProtocolInitializer();
        }
        return ServerGetProtocolInitializer();
    }

private:
    Session* session_;

    std::shared_ptr<ProtocolInitializer> ClientGetProtocolInitializer() {
        // Temporarily ensure version compatibility.
        // When all server upgrade to new version, delete this following codes.
        if (session_->config.mem_map_type == MemMapType::kDevShmFile) {
            return std::make_shared<ProtocolInitializerV2>(session_);
        }

        // Send version to peer
        Header h;
        int client_version = kMaxSupportProtoVersion;
        h.Encode(kHeaderSize, static_cast<uint8_t>(client_version), EventType::kExchangeProtoVersion);
        ProtocolTrace(h, {}, true);
        if (BlockWriteFull(session_->conn_fd, h.data_) != 0) {
            return nullptr;
        }

        // Receive peer's version
        Header recv_header;
        if (WaitEventHeader(session_->conn_fd, EventType::kExchangeProtoVersion, recv_header) != 0) {
            return nullptr;
        }

        uint8_t server_version = recv_header.Version();
        uint8_t chosen_version = static_cast<uint8_t>(std::min(client_version, static_cast<int>(server_version)));

        auto initializer = CreateProtoVersionInitializer(session_, chosen_version, nullptr);
        if (!initializer) {
            return nullptr;
        }
        return initializer;
    }

    std::shared_ptr<ProtocolInitializer> ServerGetProtocolInitializer() {
        // Ensure version compatibility
        Header h;
        if (BlockReadEventHeader(session_->conn_fd, h) != 0) {
            return nullptr;
        }

        auto initializer = CreateProtoVersionInitializer(session_, h.Version(), &h);
        if (!initializer) {
            return nullptr;
        }

        return initializer;
    }
};

std::shared_ptr<ProtocolInitializer> CreateProtoVersionInitializer(Session* session, uint8_t version, Header* first_event) {
if (auto it = protocol_version_initializers_factory.find(version); it != protocol_version_initializers_factory.end()) {
return it->second(session, first_event);
}
return nullptr;
}

int HandleShareMemoryByFilePath(Session* s, const Header& hdr) {
    s->logger->Infof("handleShareMemoryMetadata head:%s", hdr.ToString().c_str());
    std::vector<uint8_t> body(hdr.Length() - kHeaderSize);
    if (BlockReadFull(s->conn_fd, body) != 0) {
        // Todo
        if (errno != EPIPE && errno != ECONNRESET) {
            s->logger->Errorf("shmipc: Failed to read pathlen: %s", strerror(errno));
        }
        return -1;
    }
    std::string buffer_path, queue_path;
    s->ExtractShmMetadata(body, buffer_path, queue_path);
    auto qm = MappingQueueManager(queue_path);
    if (!qm) {
        s->logger->Errorf("handleShareMemoryByFilePath mappingQueueManager failed, queuePathLen:%zu path:%s err:%s",
                          queue_path.size(), queue_path.c_str(), strerror(errno));
        return -1;
    }
    s->queue_manager = qm;

    auto bm = GetGlobalBufferManager(buffer_path, 0, false, nullptr);
    if (!bm) {
        s->logger->Errorf("handleShareMemoryByFilePath mappingBufferManager failed, bufferPathLen:%zu path:%s err:%s",
                          buffer_path.size(), buffer_path.c_str(), strerror(errno));
        return -1;
    }
    s->buffer_manager = bm;

    s->handshake_done = true;
    return 0;
}

std::tuple<int, bool, std::runtime_error> HandleFallbackData(Session* s, const Header& h, const std::vector<uint8_t>& buf) {
    int event_len = static_cast<int>(h.Length());
    int payload_len = event_len - kHeaderSize;
    if (buf.size() < static_cast<size_t>(payload_len)) {
        return {0, true, std::runtime_error("")};
    }
    std::vector<uint8_t> data(buf.begin(), buf.begin() + payload_len);
    const size_t kFallbackDataHeader = 8;
    // Fallback data layout: eventHeader | seqID | status | payload
    uint32_t seq_id = be32toh(*reinterpret_cast<const uint32_t*>(data.data()));
    uint32_t status = be32toh(*reinterpret_cast<const uint32_t*>(data.data() + 4)) & 0xff;
    s->logger->Warnf("receive fallback data, length:%d seqID:%u status:%u",
                     event_len - kHeaderSize - kFallbackDataHeader, seq_id, status);
    s->OpenCircuitBreaker();
    auto fallback_slice = newBufferSlice(nullptr, std::vector<uint8_t>(data.begin() + kFallbackDataHeader, data.end()), 0, false);
    fallback_slice->writeIndex = static_cast<int>(data.size() - kFallbackDataHeader);
    std::atomic<uint64_t> fallback_read_count(0);
    fallback_read_count.fetch_add(1, std::memory_order_relaxed);
    auto stream = s->GetStream(seq_id, static_cast<StreamState>(status));
    if (!stream) {
        return {event_len, false, std::runtime_error("")};
    }
    return {event_len, false, s->HandleStreamMessage(stream, BufferSliceWrapper{fallback_slice}, static_cast<StreamState>(status))};
}

int HandleExchangeVersion(Session* s, const Header& h) {
    Header resp_header;
    resp_header.Encode(kHeaderSize, kMaxSupportProtoVersion, EventType::kExchangeProtoVersion);
    s->communication_version = static_cast<uint8_t>(std::min(static_cast<int>(h.Version()), kMaxSupportProtoVersion));
    ProtocolTrace(resp_header, {}, true);
    if (BlockWriteFull(s->conn_fd, resp_header.data_) != 0) {
        return -1;
    }
    return 0;
}

int HandleShareMemoryByMemFd(Session* s, const Header& h) {
    s->logger->Infof("recv memfd, header:%s", h.ToString().c_str());

    // 1. Receive shm metadata
    std::vector<uint8_t> body(h.Length() - kHeaderSize);
    if (BlockReadFull(s->conn_fd, body) != 0) {
        s->logger->Errorf("read shm metadata failed, reason:%s", strerror(errno));
        return -1;
    }
    std::string buffer_path, queue_path;
    s->ExtractShmMetadata(body, buffer_path, queue_path);

    // 2. Send AckReadyRecvFD
    Header ack;
    ack.Encode(kHeaderSize, s->communication_version, EventType::kAckReadyRecvFD);
    s->logger->Infof("response typeAckReadyRecvFD");
    if (BlockWriteFull(s->conn_fd, ack.data_) != 0) {
        s->logger->Errorf("send ack typeAckReadyRecvFD failed reason:%s", strerror(errno));
        return -1;
    }
    s->logger->Infof("typeAckReadyRecvFD send finished");

    // 3. Receive fd
    std::vector<uint8_t> oob(CMSG_SPACE(kMemfdCount * kMemfdDataLen));
    s->logger->Infof("send ack finished");
    ssize_t oobn = BlockReadOutOfBoundForFd(s->conn_fd, oob);
    s->logger->Infof("recvmsg finished, oob expect len:%zu, len:%zd", oob.size(), oobn);
    if (oobn != static_cast<ssize_t>(oob.size())) {
        s->logger->Errorf("handleShareMemoryByMemFd failed, reason:ReadOutOfBoundForFd, expect oobnLen:%zu, but oobnLen:%zd",
                          oob.size(), oobn);
        return -1;
    }
    auto msgs = ParseSocketControlMessage(oob);
    if (msgs.empty()) {
        s->logger->Errorf("parse socket control message ret is nil");
        return -1;
    }
    auto fds = ParseUnixRights(&msgs[0]);
    if (fds.size() < kMemfdCount) {
        s->logger->Warnf("ParseUnixRights len fds:%zu", fds.size());
        s->logger->Errorf("the number of memfd received is wrong");
        return -1;
    }

    int buffer_fd = fds[0];
    int queue_fd = fds[1];
    s->logger->Infof("recv memfd, bufferPath:%s queuePath:%s bufferFd:%d queueFd:%d",
                     buffer_path.c_str(), queue_path.c_str(), buffer_fd, queue_fd);

    // 4. Mapping share memory
    auto qm = MappingQueueManagerMemfd(queue_path, queue_fd);
    if (!qm) {
        return -1;
    }
    s->queue_manager = qm;
    auto bm = GetGlobalBufferManagerWithMemFd(buffer_path, buffer_fd, 0, false, nullptr);
    if (!bm) {
        return -1;
    }

    s->buffer_manager = bm;
    s->handshake_done = true;
    s->logger->Infof("handleShareMemoryByMemFd done");
    return 0;
}

std::tuple<int, bool, std::runtime_error> HandlePolling(Session* s, const Header& hdr, const std::vector<uint8_t>& buf) {
    std::atomic<uint64_t> recv_polling_event_count(0);
    recv_polling_event_count.fetch_add(1, std::memory_order_relaxed);
    int consumed_count = 0;
    std::runtime_error ret_err;
    while (true) {
        for (auto [ele, err] = s->queue_manager->recv_queue.pop(); !err; std::tie(ele, err) = s->queue_manager->recv_queue.pop()) {
            consumed_count++;
            StreamState state = static_cast<StreamState>(ele.status & 0xff);
            auto stream = s->GetStream(ele.seq_id, state);
            if (!stream && state == StreamState::kOpened) {
                auto slice = s->buffer_manager->ReadBufferSlice(ele.offset_in_shm_buf);
                if (!slice) {
                    return {kHeaderSize, false, std::runtime_error("Failed to read buffer slice")};
                }
                s->buffer_manager->RecycleBuffers(slice);
                continue;
            }
            if (!stream) {
                continue;
            }
            ret_err = s->HandleStreamMessage(stream, BufferSliceWrapper{.offset = ele.offset_in_shm_buf}, state);
        }

        sched_yield();
        if (s->queue_manager->recv_queue.MarkNotWorking()) {
            break;
        }
    }
    // Following code will bring runtime.convT64, which maybe result in GC.
    // s->logger->Infof("queue consumer consume size:%d on path:%s now wait", consumed_count, s->queue_manager->path.c_str());
    return {kHeaderSize, false, ret_err};
}

std::tuple<int, bool, std::runtime_error> HandleHotRestart(Session* s, const Header& hdr, const std::vector<uint8_t>& buf) {
    if (

#endif //TINYNETFLOW_PROTOCOLMANAGER_H