/*!
 * \brief per存储模块的错误码定义
 * */

#ifndef PER_KV_PER_ERR_H
#define PER_KV_PER_ERR_H

#include <cstdint>
namespace per {

enum class PerError : uint32_t {
    kStorageNotFound = 1,
    kKeyNotFound = 2,
    kIllegalWriteAccess = 3,
    kPhysicalStorageFailure = 4,
    kIntegrityCorrupted = 5,
    kValidationFailed = 6,
    kEncryptionFailed = 7,
    kDataTypeMismatch = 8,
    kInitValueNotAvailable = 9,
    kResourceBusy = 10,
    kOutOfStorageSpace = 12,
    kFileNotFound = 13,
    kNotInitialized = 14,
    // ......
};

}

#endif //PER_KV_PER_ERR_H
