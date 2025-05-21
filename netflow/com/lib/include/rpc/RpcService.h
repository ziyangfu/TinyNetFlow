

#ifndef COM_RPC_RPC_SERVICE_H
#define COM_RPC_RPC_SERVICE_H

#include "rpc/generation/rpc_service.pb.h"

/**
 * 给客户端用，服务调用方用
 * */

namespace com::rpc
{

// the meta service
class RpcServiceImpl : public RpcService
{
 public:
  RpcServiceImpl(const ServiceMap* services)
    : services_(services)
  {
  }


  virtual void listRpc(const ListRpcRequestPtr& request,
                       const ListRpcResponse* responsePrototype,
                       const RpcDoneCallback& done);

  virtual void getService(const GetServiceRequestPtr& request,
                          const GetServiceResponse* responsePrototype,
                          const RpcDoneCallback& done);

 private:

  const ServiceMap* services_;
};

}  // namespace com::rpc

#endif  // COM_RPC_RPC_SERVICE_H


// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is a public header file, it must only include public header files.