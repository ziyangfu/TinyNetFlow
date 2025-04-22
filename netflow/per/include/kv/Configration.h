/*!
 * \brief per Configration
 * */
#ifndef PER_KV_CONFIGURATION_H
#define PER_KV_CONFIGURATION_H

#include <string>
#include <optional>
#include "kv/Constants.h"
#include "kv/Types.h"

namespace per::kv {

/**
 1. kv数据库文件路径， path， 对于leveldb来说，是文件夹的路径
 2. 访问权限， 可读，可写，可读写， 若leveldb不能配置，则在per中设置检查
 3. 配置KeyValueStorageType， 默认使用leveldb
 3. 更新策略， 删除？ 覆写？
 4. kv数据库最大大小，可选设置
 5。kv数据库最小大小，可选设置
 6. 可选crc校验，默认不选，或者直接不要给出设置选项
 * */

class Configration {
public:
    std::string dbPath_;
    AccessMode accessMode_ {AccessMode::kReadWrite};
    KeyValueStorageType keyValueStorageType_ {KeyValueStorageType::kLeveldb};
    std::optional<UpdatePolicy> updatePolicy_;
    std::optional<std::uint32_t> maxKvSize_;
    std::optional<std::uint32_t> minKvSize_;
    std::optional<bool> isStorageViaCrc_;
};

}  // namespace per::kv

#endif //PER_KV_CONFIGURATION_H
