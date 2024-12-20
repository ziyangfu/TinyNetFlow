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
 *      共享内存读端
 * \file
 *      ShmReader.cpp
 * ----------------------------------------------------------------------------------------- */

#include "IO/ipc/shm/ShmReader.h"

#include <spdlog/spdlog.h>


using namespace osadaptor::ipc;

ShmReader::ShmReader()
        : sharedMemoryPath_(""),
          fd_(-1),
          mappedAddr_(nullptr),
          mappedSize_(0),
          sem_(nullptr),
          isOpen_(false),
          isRunning_(false)
{}

ShmReader::ShmReader(const std::string &sharedMemoryPath)
        : sharedMemoryPath_(sharedMemoryPath),
          fd_(-1),
          mappedAddr_(nullptr),
          mappedSize_(0),
          sem_(nullptr),
          isOpen_(false),
          isRunning_(false)
{}

ShmReader::~ShmReader() {
    close();
}


const std::string &ShmReader::getSharedMemoryFilePath() const {
    return sharedMemoryPath_;
}

int ShmReader::open() {
    if (isOpen_) {
        SPDLOG_WARN("ShmReader is already open.");
        return 0;
    }
    fd_ = shm::openSharedMemory(shmInfo_);
    if (fd_ == -1) {
        SPDLOG_ERROR("Failed to create shared memory, file: {}", sharedMemoryPath_);
        return -1;
    }
    initSemaphore();
    isOpen_ = true;
    return 0;
}

bool ShmReader::isOpen() const {
    return isOpen_;
}

void ShmReader::start() {
    if (!isOpen_) {
        SPDLOG_WARN("ShmReader is not open. Call open() first.");
        return;
    }

    if (isRunning_) {
        SPDLOG_WARN("ShmReader is already running.");
        return;
    }

    isRunning_ = true;
    SPDLOG_INFO("ShmReader started.");
}

void ShmReader::stop() {
    if (!isRunning_) {
        SPDLOG_WARN("ShmReader is not running.");
        return;
    }

    isRunning_ = false;
    SPDLOG_INFO("ShmReader stopped.");
}

void ShmReader::close() {
    if (!isOpen_) {
        SPDLOG_WARN("ShmReader is not open.");
        return;
    }
    shm::closeSharedMemoryAll(fd_, mappedAddr_, shmInfo_);

    destroySemaphore();

    isOpen_ = false;
    SPDLOG_INFO("ShmReader closed.");
}

void ShmReader::readMessage(void* buffer, size_t bufferSize) {
    if (!isRunning_) {
        SPDLOG_WARN("ShmReader is not running. Call start() first.");
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

    // Copy data from shared memory to buffer
    if (bufferSize > mappedSize_) {
        SPDLOG_WARN("Buffer size exceeds mapped size. Reading only {} bytes.", mappedSize_);
        bufferSize = mappedSize_;
    }
    memcpy(buffer, mappedAddr_, bufferSize);

    if (sem_post(sem_) == -1) {
        SPDLOG_ERROR("Failed to post semaphore.");
        return;
    }
}

void ShmReader::initSemaphore() {
    std::string semName = "/sem_" + sharedMemoryPath_;
    sem_ = shm::openSemaphore(semName.c_str(), 1);
    if (sem_ == nullptr) {
        SPDLOG_ERROR("Failed to initialize semaphore.");
    }
}

void ShmReader::destroySemaphore() {
    if (sem_ != nullptr) {
        shm::destroySemaphore(sem_);
        sem_ = nullptr;
    }
}