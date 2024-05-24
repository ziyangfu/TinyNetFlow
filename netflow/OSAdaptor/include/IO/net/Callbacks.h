/** ----------------------------------------------------------------------------------------
 * \copyright
 * Copyright (c) 2023 by the TinyNetFlow project authors. All Rights Reserved.
 *
 * This file is open source software, licensed to you under the ter；ms
 * of the Apache License, Version 2.0 (the "License").  See the NOTICE file
 * distributed with this work for additional information regarding copyright
 * ownership.  You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 * -----------------------------------------------------------------------------------------
 * \brief
 *      集合TCP上层需要的回调
 * ----------------------------------------------------------------------------------------- */

#ifndef TINYNETFLOW_OSADAPTOR_CALLBACKS_H
#define TINYNETFLOW_OSADAPTOR_CALLBACKS_H

#include <functional>
#include <memory>
#include "time/Timestamp.h"

namespace osadaptor::net {

class Buffer;
class TcpConnection;
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

using TimerCallback = std::function<void()>;
using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;
using CloseCallback = std::function<void(const TcpConnectionPtr&)>;
using WriteCompleteCallback = std::function<void(const TcpConnectionPtr&)>;
using HighWaterMarkCallback = std::function<void(const TcpConnectionPtr&, size_t)>;
using MessageCallback = std::function<void(const TcpConnectionPtr& conn, Buffer* buffer,
                                            netflow::osadaptor::time::Timestamp receiveTime)>;
//using MessageCallback = std::function<void(const TcpConnectionPtr& conn,
//                                            std::unique_ptr<Buffer> buffer,
//                                           netflow::osadaptor::time::Timestamp receiveTime )>;
/** 实现在 TcpConnection.cpp */
void defaultConnectionCallback(const TcpConnectionPtr& conn);
void defaultMessageCallback(const TcpConnectionPtr& conn,
                            Buffer* buffer,
                            netflow::osadaptor::time::Timestamp receiveTime);
//void defaultMessageCallback(const TcpConnectionPtr& conn,
//                            std::unique_ptr<Buffer> buffer,
//                            netflow::osadaptor::time::Timestamp receiveTime);

} // namespace osadaptor::net


#endif //TINYNETFLOW_OSADAPTOR_CALLBACKS_H
