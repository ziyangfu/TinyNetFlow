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
 *      共享内存写端
 * \file
 *      ShmWriter.cpp
 * ----------------------------------------------------------------------------------------- */

#include "IO/ipc/shm/ShmWriter.h"
#include "IO/ipc/internal/IpcProtocolHeader.h"

#include <spdlog/spdlog.h>

using namespace osadaptor::ipc;

ShmWriter::ShmWriter()
        : sharedMemoryPath_(""),
          fd_(-1),
          mappedAddr_(nullptr),
          mappedSize_(0),
          sem_(nullptr),
          isOpen_(false),
          isRunning_(false)
{}

ShmWriter::ShmWriter(const std::string &sharedMemoryPath)
        : sharedMemoryPath_(sharedMemoryPath),
          fd_(-1),
          mappedAddr_(nullptr),
          mappedSize_(0),
          sem_(nullptr),
          isOpen_(false),
          isRunning_(false)
{}

ShmWriter::~ShmWriter() {
    close();
}

const std::string &ShmWriter::getSharedMemoryPath() const {
    return sharedMemoryPath_;
}

int ShmWriter::open() {
    if (isOpen_) {
        SPDLOG_WARN("ShmWriter is already open.");
        return 0;
    }
    fd_ = shm::createSharedMemory(sharedMemoryInfo_);
    if (fd_ == -1) {
        SPDLOG_ERROR("Failed to create shared memory file: {}", sharedMemoryPath_);
        return -1;
    }
    initSemaphore();
    isOpen_ = true;
    return 0;
}

void ShmWriter::start() {
    if (!isOpen_) {
        SPDLOG_WARN("ShmWriter is not open. Call open() first.");
        return;
    }

    if (isRunning_) {
        SPDLOG_WARN("ShmWriter is already running.");
        return;
    }

    isRunning_ = true;
    SPDLOG_INFO("ShmWriter started.");
}

void ShmWriter::stop() {
    if (!isRunning_) {
        SPDLOG_WARN("ShmWriter is not running.");
        return;
    }

    isRunning_ = false;
    SPDLOG_INFO("ShmWriter stopped.");
}

void ShmWriter::close() {
    if (!isOpen_) {
        SPDLOG_WARN("ShmWriter is not open.");
        return;
    }
    shm::closeSharedMemoryAll(fd_, mappedAddr_, sharedMemoryInfo_);
    destroySemaphore();
    isOpen_ = false;
    SPDLOG_INFO("ShmWriter closed.");
}

void ShmWriter::writeMessage(const void* buffer, size_t bufferSize) {
    addIpcProtocolHeader();


    if (!isRunning_) {
        SPDLOG_WARN("ShmWriter is not running. Call start() first.");
        return;
    }

    if (buffer == nullptr || bufferSize == 0) {
        SPDLOG_ERROR("Invalid buffer or buffer size.");
        return;
    }

    if (sem_wait(sem_) == -1) {
        SPDLOG_ERROR("Failed to wait on semaphore.");
        return;
    }

    // Copy data from buffer to shared memory
    if (bufferSize > mappedSize_) {
        SPDLOG_WARN("Buffer size exceeds mapped size. Writing only {} bytes.", mappedSize_);
        bufferSize = mappedSize_;
    }
    memcpy(mappedAddr_, buffer, bufferSize);

    if (sem_post(sem_) == -1) {
        SPDLOG_ERROR("Failed to post semaphore.");
        return;
    }
}

void ShmWriter::initSemaphore() {
    std::string semName = "/sem_" + sharedMemoryPath_;
    sem_ = shm::openBinarySemaphore(semName.c_str());
    if (sem_ == nullptr) {
        SPDLOG_ERROR("Failed to initialize semaphore.");
    }
}

void ShmWriter::destroySemaphore() {
    if (sem_ != nullptr) {
        shm::destroySemaphore(sem_);
        sem_ = nullptr;
    }
}

/*!
 * \brief 在每条消息的前面，添加 IPC 协议头
 * */
void ShmWriter::addIpcProtocolHeader() {
    internal::IpcProtocolHeader header;
    internal::IpcProtocolHeader::serializer(header);

}

