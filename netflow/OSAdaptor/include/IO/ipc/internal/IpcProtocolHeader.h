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
 *      实现IPC通信时，payload中的协议头， 协议字段参考自字节跳动 shmipc 项目
 * \details
 * | ------------- total IPC payload ---------------|
 * |-- IPC protocol header --|-- valid payload -----|
 * |length version...    --|-- valid payload -----|
 * \file
 *      IpcProtocolHeader.h
 * ----------------------------------------------------------------------------------------- */

#ifndef OSADAPTOR_IO_IPC_INTERNAL__IPC_PROTOCOL_HEADER_H
#define OSADAPTOR_IO_IPC_INTERNAL__IPC_PROTOCOL_HEADER_H

#include <cstdint>
#include <string>
#include <cstring>
#include "spdlog/spdlog.h"

namespace osadaptor::ipc {
namespace internal {

const std::uint8_t kIpcProtocolVersion = 0x01;
const std::uint16_t kIpcProtocolMagic = 0x7758;

enum class IpcProtocolType : std::uint8_t {
    ExchangeMetadata,         /** 协议协商，交换元信息，元信息包括当前支持的 feature 列表。详见章节四：协议初始化 */
    ShareMemoryByFilePath,    /** 通过文件路径映射共享内存，详见章节四：协议初始化 */
    ShareMemoryByMemfd,       /** 通过 mmefd 映射共享内存，详见章节四：协议初始化 */
    AckReadyRecvFD,           /** 已做好准备接收 memfd，详见章节四：协议初始化 */
    AckShareMemory,           /** 完成共享内存映射，详见章节四：协议初始化 */
    SyncEvent,                /** 同步事件，用于通知对端进程处理新数据。详见章节6.1：进程同步 */
    FallbackData,             /** 共享内存不足时，通过连接发送数据。详见章节七：错误回退 */
    HotRestart,               /** 热升级，详见章节八：热升级 */
    HotRestartAck             /** 热升级完成，详见章节八：热升级 */
};

struct IpcProtocolHeader final {
public:
    std::uint32_t length_;     /** 消息总长度，包含 Header */
    std::uint16_t magic_;      /** Magic number 用于标识协议本身，固定为 0x7758 */
    std::uint8_t version_;     /** 协议版本号，用于后续迭代更新 */
    IpcProtocolType type_;        /** 消息类型 */
public:
    static std::string serializer(IpcProtocolHeader header) {
        std::string headerPayload;
        headerPayload.append(reinterpret_cast<const char*>(&header.length_), sizeof(header.length_));
        headerPayload.append(reinterpret_cast<const char*>(&header.magic_), sizeof(header.magic_));
        headerPayload.append(reinterpret_cast<const char*>(&header.version_), sizeof(header.version_));
        headerPayload.append(reinterpret_cast<const char*>(&header.type_), sizeof(header.type_));
        return headerPayload;
    }
    static IpcProtocolHeader deserializer(const std::string payload) {
        IpcProtocolHeader header;
        if (payload.size() < 8) {
            SPDLOG_ERROR("Payload is too short");
        }
        std::memcpy(&header.length_, payload.data(), sizeof(header.length_));
        std::memcpy(&header.magic_, payload.data() + 4, sizeof(header.magic_));
        std::memcpy(&header.version_, payload.data() + 6, sizeof(header.version_));
        std::memcpy(&header.type_, payload.data() + 7, sizeof(header.type_));
        return header;
//        std::istringstream iss(payload);
//        header.length_ = static_cast<std::uint32_t>(iss.get());
//        header.magic_ = static_cast<std::uint16_t>(iss.get());
//        header.version_ = static_cast<std::uint8_t>(iss.get());
//        header.type_ = static_cast<IpcProtocolType>(iss.get());
    }


};

} // namespace osadaptor::ipc
} // namespace internal

#endif // OSADAPTOR_IO_IPC_INTERNAL__IPC_PROTOCOL_HEADER_H
