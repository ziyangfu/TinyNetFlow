//
// Created by fzy on 2025/1/9.
//

#ifndef TINYNETFLOW_STREAM_H
#define TINYNETFLOW_STREAM_H

#include <cstdint>
#include <atomic>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <chrono>
#include <thread>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <system_error>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <algorithm>
#include <filesystem>
#include <future>

namespace shmipc {

enum class StreamState : uint32_t {
    kOpened = 0,
    kClosed = 1,
    kHalfClosed = 2,
};

enum class StreamCallbackState : uint32_t {
    kDefault = 0,
    kWaitExit = 1,
};

class Session;
class BufferReader;
class BufferWriter;
class BufferSlice;
class LinkedBuffer;
class PendingData;
class StreamCallbacks;
class FallbackDataEvent;
class QueueElement;

class StreamCallbacks {
public:
    virtual ~StreamCallbacks() = default;

    virtual void OnData(BufferReader reader) = 0;
    virtual void OnLocalClose() = 0;
    virtual void OnRemoteClose() = 0;
};

class Stream {
public:
    Stream(Session* session, uint32_t id)
            : id_(id),
              session_(session),
              state_(static_cast<uint32_t>(StreamState::kOpened)),
              recv_buf_(new LinkedBuffer(session->buffer_manager)),
              send_buf_(new LinkedBuffer(session->buffer_manager)),
              pending_data_(new PendingData(this)),
              recv_notify_ch_(),
              close_notify_ch_(),
              in_fallback_state_(false),
              callback_(nullptr),
              callback_in_process_(0),
              callback_close_state_(static_cast<uint32_t>(StreamCallbackState::kDefault)) {
        recv_buf_->BindStream(this);
        send_buf_->BindStream(this);
        pending_data_->SetStream(this);
    }

    ~Stream() {
        Close();
    }

    std::error_code SetCallbacks(StreamCallbacks* callback) {
        if (GetCallbacks() != nullptr) {
            return std::make_error_code(std::errc::operation_not_permitted);
        }
        callback_ = callback;
        callback_in_process_ = 0;
        return std::error_code();
    }

    Session* Session() const {
        return session_;
    }

    uint32_t StreamID() const {
        return id_;
    }

    std::error_code ReadMore(size_t min_size) {
        pending_data_->MoveTo(recv_buf_.get());
        size_t recv_len = recv_buf_->Len();
        if (recv_len >= min_size) {
            return std::error_code();
        }

        if (recv_len == 0 && !IsOpen()) {
            return std::make_error_code(std::errc::bad_file_descriptor);
        }

        std::chrono::steady_clock::time_point deadline;
        if (!read_deadline_.time_since_epoch().count()) {
            deadline = std::chrono::steady_clock::time_point::max();
        } else {
            deadline = read_deadline_;
        }

        std::unique_lock<std::mutex> lock(mutex_);
        while (true) {
            if (recv_len >= min_size) {
                return std::error_code();
            }

            if (!IsOpen()) {
                if (recv_len >= min_size) {
                    return std::error_code();
                }
                if (GetStreamState() == static_cast<uint32_t>(StreamState::kHalfClosed)) {
                    return std::make_error_code(std::errc::bad_file_descriptor);
                }
                return std::make_error_code(std::errc::connection_aborted);
            }

            std::cv_status status = recv_cv_.wait_until(lock, deadline);
            if (status == std::cv_status::timeout) {
                return std::make_error_code(std::errc::timed_out);
            }

            pending_data_->MoveTo(recv_buf_.get());
            recv_len = recv_buf_->Len();
        }
    }

    BufferWriter BufferWriter() {
        return BufferWriter(send_buf_.get());
    }

    BufferReader BufferReader() {
        return BufferReader(recv_buf_.get());
    }

    std::error_code Flush(bool end_stream) {
        if (send_buf_->Len() == 0) {
            return std::error_code();
        }

        std::atomic_fetch_add(&session_->stats.out_flow_bytes, send_buf_->Len());
        uint32_t state = GetStreamState();
        if (state != static_cast<uint32_t>(StreamState::kOpened)) {
            send_buf_->Recycle();
            return std::make_error_code(std::errc::connection_aborted);
        }

        send_buf_->Done(end_stream);
        send_buf_->Clean();
        if (!send_buf_->IsFromShareMemory()) {
            in_fallback_state_ = true;
        }

        if (in_fallback_state_) {
            return WriteFallback(state, std::make_error_code(std::errc::no_buffer_space));
        }

        QueueElement element{
                .seq_id = id_,
                .offset_in_shm_buf = send_buf_->RootBufOffset(),
                .status = state,
        };

        auto err = session_->SendQueue()->Put(element);
        if (err == std::make_error_code(std::errc::device_or_resource_busy)) {
            std::atomic_fetch_add(&session_->stats.queue_full_error_count, 1);
            auto write_deadline = write_deadline_;
            if (!write_deadline.time_since_epoch().count()) {
                write_deadline = std::chrono::steady_clock::now() + std::chrono::seconds(1);
            }

            for (int i = 0; i < 10 && err == std::make_error_code(std::errc::device_or_resource_busy); ++i) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                err = session_->SendQueue()->Put(element);
                if (std::chrono::steady_clock::now() >= write_deadline) {
                    err = std::make_error_code(std::errc::timed_out);
                }
            }
        }

        if (err) {
            send_buf_->Recycle();
            return err;
        }

        return session_->WakeUpPeer();
    }

    std::error_code WriteFallback(uint32_t stream_status, const std::error_code& err) {
        session_->logger.Warnf("stream fallback seqID:%u len:%zu reason:%s, sendBuf.isFromShareMemory: %d",
                               id_, send_buf_->Len(), err.message().c_str(), send_buf_->IsFromShareMemory());

        FallbackDataEvent event;
        event.Encode(sizeof(event) + send_buf_->Len(), session_->communication_version, id_, stream_status);
        std::vector<uint8_t> data;
        data.reserve(send_buf_->Len() + sizeof(event));
        data.insert(data.end(), event.data(), event.data() + sizeof(event));
        auto underlying_slices = send_buf_->UnderlyingData();
        for (const auto& slice : underlying_slices) {
            data.insert(data.end(), slice.begin(), slice.end());
        }

        send_buf_->Recycle();
        session_->OpenCircuitBreaker();
        std::atomic_fetch_add(&session_->stats.fallback_write_count, 1);
        return session_->WaitForSend(nullptr, data);
    }

    std::error_code Close() {
        if (GetCallbacks() != nullptr) {
            callback_close_state_ = static_cast<uint32_t>(StreamCallbackState::kWaitExit);
        }

        if (std::atomic_load(&callback_in_process_) == 1) {
            std::atomic_compare_exchange_weak(&state_, static_cast<uint32_t>(StreamState::kOpened),
                                              static_cast<uint32_t>(StreamState::kHalfClosed));
            return std::error_code();
        }

        return CloseImpl();
    }

    std::error_code CloseImpl() {
        uint32_t old_state = GetStreamState();
        if (old_state == static_cast<uint32_t>(StreamState::kClosed)) {
            return std::error_code();
        }

        if (std::atomic_compare_exchange_weak(&state_, old_state, static_cast<uint32_t>(StreamState::kClosed))) {
            if (GetCallbacks() != nullptr) {
                async_goroutine_wg_.wait();
            }

            Clean();
            if (old_state == static_cast<uint32_t>(StreamState::kOpened)) {
                SafeCloseNotify();
                auto callback = GetCallbacks();
                if (callback != nullptr) {
                    if (session_->IsClosed()) {
                        callback->OnRemoteClose();
                    } else {
                        callback->OnLocalClose();
                    }
                }

                if (session_->IsClosed()) {
                    return std::error_code();
                }

                QueueElement element{
                        .seq_id = id_,
                        .status = static_cast<uint32_t>(StreamState::kClosed),
                };

                auto err = session_->SendQueue()->Put(element);
                if (err) {
                    std::atomic_fetch_add(&session_->stats.queue_full_error_count, 1);
                    std::array<uint8_t, 12> stream_close_event;
                    Header(stream_close_event.data()).Encode(12, session_->communication_version, EventType::kStreamClose);
                    binary::Encode(stream_close_event.data() + 8, id_);
                    return session_->WaitForSend(nullptr, stream_close_event);
                }

                return session_->WakeUpPeer();
            }
        }

        return std::error_code();
    }

    void Clean() {
        session_->OnStreamClose(id_, static_cast<StreamState>(GetStreamState()));
        pending_data_->Clear();
        recv_buf_->Recycle();
        send_buf_->Recycle();
    }

    void HalfClose() {
        if (std::atomic_compare_exchange_weak(&state_, static_cast<uint32_t>(StreamState::kOpened),
                                              static_cast<uint32_t>(StreamState::kHalfClosed))) {
            SafeCloseNotify();
            auto callback = GetCallbacks();
            if (callback != nullptr) {
                callback->OnRemoteClose();
            }
        }
    }

    std::error_code Reset() {
        if (!IsOpen()) {
            return std::make_error_code(std::errc::connection_aborted);
        }

        size_t unread_size = recv_buf_->Len();
        if (unread_size > 0) {
            return std::make_error_code(std::errc::invalid_argument);
        }

        pending_data_->Lock();
        if (!pending_data_->IsEmpty()) {
            pending_data_->Unlock();
            return std::make_error_code(std::errc::invalid_argument);
        }
        pending_data_->Unlock();

        read_deadline_ = std::chrono::steady_clock::time_point();
        write_deadline_ = std::chrono::steady_clock::time_point();
        in_fallback_state_ = false;

        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!recv_notify_ch_.empty()) {
                recv_notify_ch_.pop();
            }
        }

        SetCallbacks(nullptr);
        return std::error_code();
    }

    void ReleaseReadAndReuse() {
        recv_buf_->ReleasePreviousReadAndReserve();
        if (recv_buf_->Len() == 0 && recv_buf_->SliceListSize() == 1) {
            std::swap(recv_buf_, send_buf_);
        }
    }

    std::error_code FillDataToReadBuffer(const BufferSliceWrapper& buf) {
        pending_data_->Add(buf);
        if (GetStreamState() == static_cast<uint32_t>(StreamState::kClosed)) {
            pending_data_->Clear();
            recv_buf_->Recycle();
            return std::error_code();
        }

        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!recv_notify_ch_.empty()) {
                recv_notify_ch_.pop();
            }
        }

        auto callback = GetCallbacks();
        if (callback != nullptr) {
            if (std::atomic_compare_exchange_weak(&callback_in_process_, 0, 1)) {
                async_goroutine_wg_.add(1);
                std::thread([this, callback]() {
                    while (true) {
                        pending_data_->MoveTo(recv_buf_.get());
                        while (IsOpen() && recv_buf_->Len() > 0) {
                            callback->OnData(BufferReader(recv_buf_.get()));
                            pending_data_->MoveTo(recv_buf_.get());
                        }

                        std::atomic_store(&callback_in_process_, 0);
                        if (std::atomic_load(&callback_close_state_) == static_cast<uint32_t>(StreamCallbackState::kWaitExit)) {
                            async_goroutine_wg_.done();
                            CloseImpl();
                            return;
                        }

                        if (!(pending_data_->HasUnread() && std::atomic_compare_exchange_weak(&callback_in_process_, 0, 1))) {
                            break;
                        }
                    }
                    async_goroutine_wg_.done();
                }).detach();
            }
        }

        return std::error_code();
    }

    std::error_code SetDeadline(const std::chrono::steady_clock::time_point& t) {
        read_deadline_ = t;
        write_deadline_ = t;
        return std::error_code();
    }

    std::error_code SetReadDeadline(const std::chrono::steady_clock::time_point& t) {
        read_deadline_ = t;
        return std::error_code();
    }

    std::error_code SetWriteDeadline(const std::chrono::steady_clock::time_point& t) {
        write_deadline_ = t;
        return std::error_code();
    }

    bool IsOpen() const {
        return GetStreamState() == static_cast<uint32_t>(StreamState::kOpened);
    }

    uint32_t GetStreamState() const {
        return std::atomic_load(&state_);
    }

    void SafeCloseNotify() {
        std::call_once(close_notify_once_, [this]() {
            close_notify_ch_.close();
        });
    }

private:
    uint32_t id_;
    Session* session_;
    std::atomic<uint32_t> state_;
    std::unique_ptr<LinkedBuffer> recv_buf_;
    std::unique_ptr<LinkedBuffer> send_buf_;
    std::unique_ptr<PendingData> pending_data_;
    std::queue<std::promise<void>> recv_notify_ch_;
    std::promise<void> close_notify_ch_;
    std::mutex mutex_;
    std::condition_variable recv_cv_;
    std::chrono::steady_clock::time_point read_deadline_;
    std::chrono::steady_clock::time_point write_deadline_;
    bool in_fallback_state_;
    StreamCallbacks* callback_;
    std::atomic<uint32_t> callback_in_process_;
    std::atomic<uint32_t> callback_close_state_;
    std::atomic_flag close_notify_once_ = ATOMIC_FLAG_INIT;
    std::jthread async_goroutine_wg_;

    void SetCallbacks(StreamCallbacks* sc) {
        callback_ = sc;
    }

    StreamCallbacks* GetCallbacks() const {
        return callback_;
    }

    int CopyRead(uint8_t* p, size_t len) {
        return recv_buf_->Read(p, len);
    }

    int CopyWriteAndFlush(const uint8_t* p, size_t len) {
        return send_buf_->CopyWriteAndFlush(p, len);
    }
};

class BufferReader {
public:
    explicit BufferReader(LinkedBuffer* buffer) : buffer_(buffer) {}

    int Read(uint8_t* p, size_t len) {
        return buffer_->Read(p, len);
    }

private:
    LinkedBuffer* buffer_;
};

class BufferWriter {
public:
    explicit BufferWriter(LinkedBuffer* buffer) : buffer_(buffer) {}

    int Write(const uint8_t* p, size_t len) {
        return buffer_->Write(p, len);
    }

private:
    LinkedBuffer* buffer_;
};

class BufferSlice {
public:
    BufferSlice(const std::vector<uint8_t>& data) : data_(data) {}

    const uint8_t* Data() const {
        return data_.data();
    }

    size_t Size() const {
        return data_.size();
    }

    bool HasNext() const {
        // Implementation to check if there is a next buffer slice
        return false;
    }

    uint32_t NextBufferOffset() const {
        // Implementation to get the next buffer offset
        return 0;
    }

private:
    std::vector<uint8_t> data_;
};

class LinkedBuffer {
public:
    explicit LinkedBuffer(/* BufferManager* buffer_manager */) {}

    void BindStream(Stream* stream) {
        // Implementation to bind stream
    }

    size_t Len() const {
        // Implementation to get the length of the buffer
        return 0;
    }

    void Recycle() {
        // Implementation to recycle the buffer
    }

    void Done(bool end_stream) {
        // Implementation to mark the buffer as done
    }

    void Clean() {
        // Implementation to clean the buffer
    }

    bool IsFromShareMemory() const {
        // Implementation to check if the buffer is from shared memory
        return false;
    }

    uint32_t RootBufOffset() const {
        // Implementation to get the root buffer offset
        return 0;
    }

    std::vector<std::vector<uint8_t>> UnderlyingData() const {
        // Implementation to get the underlying data slices
        return {};
    }

    void AppendBufferSlice(const BufferSlice& slice) {
        // Implementation to append a buffer slice
    }

    void ReleasePreviousReadAndReside() {
        // Implementation to release previous read and reserve
    }

    size_t SliceListSize() const {
        // Implementation to get the size of the slice list
        return 0;
    }

    int Read(uint8_t* p, size_t len) {
        // Implementation to read data from the buffer
        return 0;
    }

    int Write(const uint8_t* p, size_t len) {
        // Implementation to write data to the buffer
        return 0;
    }

    int CopyWriteAndFlush(const uint8_t* p, size_t len) {
        // Implementation to copy data and flush
        return 0;
    }

private:
    // Members and methods
};

class PendingData {
public:
    explicit PendingData(Stream *stream) : stream_(stream) {}

    void MoveToWithoutLock(LinkedBuffer *to_buf) {
        if (unread_.empty()) {
            return;
        }

        size_t pre_len = to_buf->Len();
        for (const auto &w: unread_) {
            if (w.fallback_slice != nullptr) {
                to_buf->AppendBuffer
            }
        }
    }
};



#endif //TINYNETFLOW_STREAM_H