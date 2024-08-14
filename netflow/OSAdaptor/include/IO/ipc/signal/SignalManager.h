/** ----------------------------------------------------------------------------------------
 * \copyright
 * Copyright (c) 2024 by the TinyNetFlow project authors. All Rights Reserved.
 *
 * This file is open source software, licensed to you under the ter；ms
 * of the Apache License, Version 2.0 (the "License").  See the NOTICE file
 * distributed with this work for additional information regarding copyright
 * ownership.  You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 * -----------------------------------------------------------------------------------------
 * \brief
 *      信号处理
 * \file
 *      SignalManager.h
 * ----------------------------------------------------------------------------------------- */

#ifndef TINYNETFLOW_OSADAPTOR_SIGNALMANAGER_H
#define TINYNETFLOW_OSADAPTOR_SIGNALMANAGER_H

#include <csignal>
#include <map>
#include <functional>

/*!
 * 1. 使用 signalfd 创建信号描述符， channel持有fd？
 * 2. 将signalfd添加到reactor，注册读事件
 * 3. 触发读事件时，执行对应的回调函数
 * */
namespace osadaptor::ipc {
namespace signal {

class SignalManager final {
private:
    using SignalHandler = std::function<void ()>;
    std::map<int, SignalHandler> signalCallbackHandlers_; //! 信号以及信号的回调函数
    int signal_fd_;
public:
    SignalManager();
    ~SignalManager();
    SignalManager(SignalManager const&) = delete;
    SignalManager& operator=(SignalManager const&) = delete;
    SignalManager(SignalManager&&) = delete;
    SignalManager& operator=(SignalManager&&) = delete;
    void init();
private:
    static void defaultSignalHandler(int);
    void setSignalCallback(SignalHandler handler);
    void resetSignalHandler();
    void closeSignalFd();
    bool isSignalAllowed(int signal); /** 有些信号不允许 */

    /** EventLoop* loop_; */
    /** reactor callback */



};

}  // namespace signal


}

#endif TINYNETFLOW_OSADAPTOR_SIGNALMANAGER_H
