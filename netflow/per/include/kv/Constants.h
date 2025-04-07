
#ifndef TINYNETFLOW_CONSTANTS_H
#define TINYNETFLOW_CONSTANTS_H

#include <string>

const std::string kvStorageFileName = "netflow_per_kvdb";

/**

 1. kv数据库文件路径， path， 对于leveldb来说，是文件夹的路径
 2. 访问权限， 可读，可写，可读写， 若leveldb不能配置，则在per中设置检查
 3. 配置KeyValueStorageType， 默认使用leveldb
 3. 更新策略， 删除？ 覆写？
 4. kv数据库最大大小，可选设置
 5。kv数据库最小大小，可选设置
 6. 可选crc校验，默认不选，或者直接不要给出设置选项
 *
 * */

#endif //TINYNETFLOW_CONSTANTS_H
