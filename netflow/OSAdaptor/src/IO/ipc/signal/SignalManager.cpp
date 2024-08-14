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
 *      SignalManager.cpp
 * ----------------------------------------------------------------------------------------- */
#include "IO/ipc/signal/SignalManager.h"

#include <spdlog/spdlog.h>
#include <csignal>
#include <sys/signalfd.h>

namespace osadaptor::ipc {
namespace signal {
/*!
 * \brief 默认的信号回调函数，不会调用
 * */
/* static */ void SignalManager::defaultSignalHandler(int) {
    SPDLOG_ERROR("default signal handler error");
}

SignalManager::SignalManager()
    : signalCallbackHandlers_(),
      signal_fd_(-1)
{

}
/*!
 * \details 关闭 signalfd，移除注册reactor
 * */
SignalManager::~SignalManager() {
    closeSignalFd();

}

void SignalManager::init() {
    sigset_t sig_set;
    if (sigemptyset(&sig_set) != 0) {
        SPDLOG_ERROR("SignalManager: signal empty init failure");
    }
    for (auto it : signalCallbackHandlers_) {
        int signal = it.first;
        if (!isSignalAllowed(signal)) {
            SPDLOG_ERROR("SignalManager: it contain illegal signal in container");
            break;
        }
        if (sigaddset(&sig_set, signal) != 0) {
            SPDLOG_ERROR("SignalManager: failed to add signal to sigset");
        }


        signal_fd_ = signalfd(-1, &sig_set, SFD_CLOEXEC | SFD_NONBLOCK);

    }
}

void SignalManager::setSignalCallback(SignalHandler handler) {
    int sig{0};
    struct sigaction action{};
    action.sa_handler = &defaultSignalHandler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(sig, &action, nullptr);
}

void SignalManager::closeSignalFd() {
    if (signal_fd_ != -1) {
        close(signal_fd_);
    }
}

bool SignalManager::isSignalAllowed(int signal) {
    return (signal != SIGILL)   &&     /** 非法硬件指令 */
           (signal != SIGBUS)   &&     /** 硬件故障 */
           (signal != SIGFPE)   &&     /** 算术异常 */
           (signal != SIGKILL)  &&     /** 终止 */
           (signal != SIGSEGV)  &&     /** 无效内存引用 */
           (signal != SIGSTOP)  &&     /** 强制性的进程停止信号 */
           (signal != SIGUSR1);        /**  用户定义信号 */
}
} // namespace signal

} // namespace osadaptor::ipc