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
 *      one loop per thread, 为EventLoop所有者
 * \file
 *      EventLoopThread.cpp
 * ----------------------------------------------------------------------------------------- */

#include "IO/reactor/EventLoopThread.h"
#include "IO/reactor/EventLoop.h"

#include <spdlog/spdlog.h>
#include <cassert>

using namespace netflow::osadaptor::net;
/*!
 * \brief EventLoopThread构造函数
 * \param cb： 线程初始化时的回调函数，可有可无
 * \param name: 线程名称， 暂时没有用到  */
EventLoopThread::EventLoopThread(const EventLoopThread::ThreadInitCallback &cb,
                                 const std::string &name)
                 : loop_(nullptr),
                   exiting_(false),
                   callback_(cb),
                   ready_(false)
{
    SPDLOG_TRACE("EventLoopThread constructor");
}

EventLoopThread::~EventLoopThread() {
    SPDLOG_TRACE("EventLoopThread dtor");
    exiting_ = true;
    if (loop_) {
        loop_->quit();
        thread_->join();
        //join();
    }
    if (thread_ && thread_->joinable()) {
        thread_->join();
    }
}
/**
 * \brief 主线程，启动线程
 * \return IO线程 loop shared_ptr指针 */
std::shared_ptr<EventLoop> EventLoopThread::startLoop() {
    SPDLOG_TRACE("Start loop thread");
    assert(thread_.get() == nullptr);
    thread_.reset(new std::thread(std::bind(&EventLoopThread::threadFunc, this)));

    std::shared_ptr<EventLoop> loop = nullptr;

    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (loop_ == nullptr){
            /** 避免虚假唤醒，只有条件ready_变为true时才可以唤醒 */
            cond_.wait(lock, [this]{
                return ready_;
            });
        }
        loop = loop_;
    }
    return loop;
}
/**
 * \brief 子线程，启动loop，并把loop的地址给了主线程
 * */
void EventLoopThread::threadFunc() {
    std::shared_ptr<EventLoop> loop = std::make_shared<EventLoop>();
    if(callback_){
        callback_(loop);
    }
    {
        std::unique_lock<std::mutex> lock(mutex_);
        ready_ = true;
        loop_ = loop;
        cond_.notify_one();
    }
    loop->loop();  /** 开始循环 */

    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = nullptr;
}

void EventLoopThread::join() {
    std::unique_lock<std::mutex> lock(mutex_);
    if (thread_ && thread_->joinable()) {
        SPDLOG_TRACE("thread ID {} joinable", thread_->get_id());
        try {
            thread_->join();
        }
        catch (const std::system_error& e) {
            SPDLOG_ERROR("caught a system error: {}, the code is {}", e.what(), e.code().message());
        }
        thread_.reset();
    }

}