// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "rpc/RpcCodec.h"

#include "IO/net/Endian.h"
#include "IO/net/TcpConnection.h"

#include "rpc/generation/rpc.pb.h"
#include "rpc/google-inl.h"

using namespace com::rpc;
using namespace osadaptor::net;
using namespace osadaptor::time;

namespace
{
  int ProtobufVersionCheck()
  {
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    return 0;
  }
  int dummy __attribute__ ((unused)) = ProtobufVersionCheck();
}

namespace com::rpc {
const char rpctag [] = "RPC0";
}
