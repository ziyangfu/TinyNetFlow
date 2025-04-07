//
// Created by fzy on 2025/4/7.
//

#ifndef TINYNETFLOW_TYPES_H
#define TINYNETFLOW_TYPES_H

#include <cstdint>

namespace per::kv {

/*!
 * \brief 配置数据库的访问权限
 * */
enum class AccessMode : std::uint8_t {
    kRead,
    kWrite,
    kReadWrite
};
/*!
 * \brief 配置使用何种数据库，默认将使用leveldb
 * \details 若使用map，则磁盘存储格式为json，直接明文存储，不配置为加密存储
 * */
enum class KeyValueStorageType : std::uint8_t {
    kLeveldb,
    kMap
};

/** 一些bool判断，例如isReadable，isWriteable，isReadWriteable */

} // namespace per::kv

#endif //TINYNETFLOW_TYPES_H
