/*!
 * \brief 对发送与接收到的字节流数据进行编解码
 * */
#ifndef COM_RPC_PROTOBUF_CODEC_LITE_H
#define COM_RPC_PROTOBUF_CODEC_LITE_H

//#include "muduo/base/noncopyable.h"
//#include "muduo/base/StringPiece.h"
#include "time/Timestamp.h"
#include "IO/net/Callbacks.h"

// temp
#include "rpc/RpcChannel.h"

#include <string>
#include <memory>
#include <type_traits>

namespace google::protobuf
{
class Message;
}

namespace com::rpc
{
using MessagePtr = std::shared_ptr<google::protobuf::Message>;

// wire format
//
// Field     Length  Content
//
// size      4-byte  M+N+4
// tag       M-byte  could be "RPC0", etc.
// payload   N-byte
// checksum  4-byte  adler32 of tag+payload
//
// This is an internal class, you should use ProtobufCodecT instead.
class ProtobufCodecLite
{
 public:
  const static int kHeaderLen = sizeof(int32_t);
  const static int kChecksumLen = sizeof(int32_t);
  const static int kMaxMessageLen = 64*1024*1024; // same as codec_stream.h kDefaultTotalBytesLimit

  enum class ErrorCode : std::uint8_t
  {
    kNoError = 0,
    kInvalidLength,
    kCheckSumError,
    kInvalidNameLen,
    kUnknownMessageType,
    kParseError,
  };

  // return false to stop parsing protobuf message
  using RawMessageCallback = std::function<bool (const osadaptor::net::TcpConnectionPtr&,
                                                 std::string,
                                                 osadaptor::time::Timestamp)>;
  using ProtobufMessageCallback = std::function<void (const osadaptor::net::TcpConnectionPtr&,
                                                      const MessagePtr&,
                                                      osadaptor::time::Timestamp)>;

  using ErrorCallback = std::function<void (const osadaptor::net::TcpConnectionPtr&,
                                            osadaptor::net::Buffer*,
                                            osadaptor::time::Timestamp,
                                            ErrorCode)>;

  ProtobufCodecLite(const ::google::protobuf::Message* prototype,
                    std::string tagArg,
                    const ProtobufMessageCallback& messageCb,
                    const RawMessageCallback& rawCb = RawMessageCallback(),
                    const ErrorCallback& errorCb = defaultErrorCallback)
    : prototype_(prototype),
      tag_(tagArg),
      messageCallback_(messageCb),
      rawCb_(rawCb),
      errorCallback_(errorCb),
      kMinMessageLen(tagArg.size() + kChecksumLen)
  {
  }

  virtual ~ProtobufCodecLite() = default;

  const std::string& tag() const { return tag_; }
  /*!
   * \brief 发将给定的消息编码并发送到指定的TCP连接
   * */
  void send(const osadaptor::net::TcpConnectionPtr& conn,
            const ::google::protobuf::Message& message);
  /*!
   * \brief 处理接收到的消息，解析缓冲区中的数据并调用相应的回调
   * */
  void onMessage(const osadaptor::net::TcpConnectionPtr& conn,
                 osadaptor::net::Buffer* buf,
                 osadaptor::time::Timestamp receiveTime);
  /*!
   * \brief 从字符串缓冲区中解析出消息
   * */
  virtual bool parseFromBuffer(std::string buf, google::protobuf::Message* message);
  /*!
   * \brief 将消息序列化到缓冲区中
   * */
  virtual int serializeToBuffer(const google::protobuf::Message& message,
                                osadaptor::net::Buffer* buf);

  static const std::string& errorCodeToString(ErrorCode errorCode);

  // public for unit tests
  ErrorCode parse(const char* buf, int len, ::google::protobuf::Message* message);
  void fillEmptyBuffer(osadaptor::net::Buffer* buf, const google::protobuf::Message& message);

  static int32_t checksum(const void* buf, int len);
  static bool validateChecksum(const char* buf, int len);
  static int32_t asInt32(const char* buf);
  static void defaultErrorCallback(const osadaptor::net::TcpConnectionPtr&,
                                   osadaptor::net::Buffer*,
                                   osadaptor::time::Timestamp,
                                   ErrorCode);

 private:
  const ::google::protobuf::Message* prototype_;
  const std::string tag_;
  ProtobufMessageCallback messageCallback_;
  RawMessageCallback rawCb_;
  ErrorCallback errorCallback_;
  const int kMinMessageLen;
};

// TAG must be a variable with external linkage, not a string literal
template<typename MSG, const char* TAG, typename CODEC=ProtobufCodecLite>
class ProtobufCodecLiteT
{
  static_assert(std::is_base_of<ProtobufCodecLite, CODEC>::value, "CODEC should be derived from ProtobufCodecLite");
 public:
    using ConcreteMessagePtr = std::shared_ptr<MSG>;
    using ProtobufMessageCallback = std::function<void (const osadaptor::net::TcpConnectionPtr&,
                                                        const ConcreteMessagePtr&,
                                                        osadaptor::time::Timestamp)>;
    using RawMessageCallback = ProtobufCodecLite::RawMessageCallback;
    using ErrorCallback = ProtobufCodecLite::ErrorCallback;

  explicit ProtobufCodecLiteT(const ProtobufMessageCallback& messageCb,
                              const RawMessageCallback& rawCb = RawMessageCallback(),
                              const ErrorCallback& errorCb = ProtobufCodecLite::defaultErrorCallback)
    : messageCallback_(messageCb),
      codec_(&MSG::default_instance(),
             TAG,
             std::bind(&ProtobufCodecLiteT::onRpcMessage, this, std::placeholders::_1,
                       std::placeholders::_2, std::placeholders::_3),
             rawCb,
             errorCb)
  {
  }

  const std::string& tag() const { return codec_.tag(); }

  void send(const osadaptor::net::TcpConnectionPtr& conn,
            const MSG& message)
  {
    codec_.send(conn, message);
  }

  void onMessage(const osadaptor::net::TcpConnectionPtr& conn,
                 osadaptor::net::Buffer* buf,
                 osadaptor::time::Timestamp receiveTime)
  {
    codec_.onMessage(conn, buf, receiveTime);
  }

  // internal
  void onRpcMessage(const osadaptor::net::TcpConnectionPtr& conn,
                    const MessagePtr& message,
                    osadaptor::time::Timestamp receiveTime)
  {
    messageCallback_(conn, ::google::protobuf::down_pointer_cast<MSG>(message), receiveTime);
  }

  void fillEmptyBuffer(osadaptor::net::Buffer* buf, const MSG& message)
  {
    codec_.fillEmptyBuffer(buf, message);
  }

 private:
  ProtobufMessageCallback messageCallback_;
  CODEC codec_;
};

}  // namespace com::rpc

#endif  // COM_RPC_PROTOBUF_CODEC_LITE_H


// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is a public header file, it must only include public header files.

// For Protobuf codec supporting multiple message types, check
// examples/protobuf/codec