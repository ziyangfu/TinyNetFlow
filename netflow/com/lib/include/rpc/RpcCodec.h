// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is a public header file, it must only include public header files.

#ifndef COM_RPC_RPC_CODEC_H
#define COM_RPC_RPC_CODEC_H

#include "time/Timestamp.h"
#include "rpc/ProtobufCodecLite.h"

namespace com::rpc
{

class Buffer;
class TcpConnection;
typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

class RpcMessage;
typedef std::shared_ptr<RpcMessage> RpcMessagePtr;
extern const char rpctag[];// = "RPC0";

// wire format
//
// Field     Length  Content
//
// size      4-byte  N+8
// "RPC0"    4-byte
// payload   N-byte
// checksum  4-byte  adler32 of "RPC0"+payload
//

using RpcCodec = ProtobufCodecLiteT<RpcMessage, rpctag>;

}  // namespace com::rpc

#endif  // COM_RPC_RPC_CODEC_H
