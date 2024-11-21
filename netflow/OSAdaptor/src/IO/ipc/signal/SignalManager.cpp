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
      signal_fd_(-1),
      loop_(nullptr),
      signal_fd_channel_(nullptr)
{

}
/*!
 * \details 关闭 signalfd，移除注册reactor
 * */
SignalManager::~SignalManager() {
    closeSignalFd();

}

/*!
 * \details
 *  1. signalfd_siginfo 是一个结构体，用于在 signalfd 机制中传递信号信息。当信号到达时，signalfd 文件描述符变为可读状态，
 *      读取该文件描述符可以获得一个signalfd_siginfo 结构体实例，其中包含了关于信号的详细信息
 * */
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

void SignalManager::setDefaultSignalHandler(int signal) {
    struct sigaction action{};
    //! 信号回调函数， default回调函数正常情况下是不会触发的
    action.sa_handler = &defaultSignalHandler;
    if (sigemptyset(&action.sa_mask) != 0) {
        SPDLOG_ERROR("SignalManager: failed to set signal");
    }
    action.sa_flags = 0; //! 表示没有设置任何标识
    if (sigaction(signal, &action, nullptr) != 0) {
        SPDLOG_ERROR("SignalManager: failed to set default signal handler");
    }
}

void SignalManager::removeDefaultSignalHandler() {
    struct sigaction action{};
    action.sa_handler = SIG_DFL;
    if (sigemptyset(&action.sa_mask) != 0) {
        SPDLOG_ERROR("SignalManager: failed to set signal");
    }
    action.sa_flags = 0;
    for (auto it : signalCallbackHandlers_) {
        int signal = it.first;
        if (isSignalAllowed(signal)) {
            if (sigaction(signal, &action, nullptr) != 0) {
                SPDLOG_ERROR("SignalManager: failed to remove default signal handler");
            }
        }
    }
}
/*!
 * \details
 *
 * */
void SignalManager::setSignalCallback(SignalHandler handler) {
    signal_fd_channel_ = std::make_unique<net::Channel>(loop_, signal_fd_); /** FIXME */
    signal_fd_channel_->enableReading();
    signal_fd_channel_->setReadCallback([this](time::Timestamp){
        struct signalfd_siginfo sig_info{};
        constexpr int kMaxCycle = 32;
        int i = 0;
        while (i < kMaxCycle) {
            if (read(signal_fd_, &sig_info, sizeof(sig_info)) == -1) {
                if (errno == EAGAIN) {
                    /** 在一个非阻塞的文件描述符上尝试执行读或写操作，但此时没有数据可读或写缓冲区已满时，
                     * 系统调用如 read() 或 write() 将返回 -1 并设置 errno 为 EAGAIN,意味着没有更多的信号事件可以处理，
                     * 因此通过 break 语句退出循环 */
                    break;
                }
                else {
                    SPDLOG_ERROR("SignalManager: error in signal callback handler");
                }
            }
            //! 通过signal.ssi_signo获取信号编号，根据编号从map中取出回调函数，并执行
            signalCallbackHandlers_[static_cast<int>(sig_info.ssi_signo)];
            i++;
        }
    });



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