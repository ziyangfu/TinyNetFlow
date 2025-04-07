/*!
 * \brief per 存储模块的kv存储接口，参考了 AUTOSAR AP persistency（一定程度上）
 * */
#ifndef PER_KV_KEY_VALUE_STORAGE_H
#define PER_KV_KEY_VALUE_STORAGE_H

#include <string>
#include <vector>
#include <memory>
#include <cstdint>
#include <leveldb/db.h>

namespace per::kv {


/*!
 * 1. 写入kv
 * 2. 读取kv
 * 3. 删除kv
 * 4. 批量写入
 * 5. 遍历数据
 * 6. 快照 snapshot
 * 7. 压缩数据（可选）
 * */
class KeyValueStorage final {
public:
    KeyValueStorage(const KeyValueStorage &) = delete;
    KeyValueStorage& operator=(const KeyValueStorage &) = delete;

    KeyValueStorage(KeyValueStorage &&) = default;
    KeyValueStorage& operator=(KeyValueStorage &&) = default;

    void init(const std::string& db_path);
    /*!
     * \brief 获取所有的key
     * \return 所有的key的列表
     * */
    std::vector<std::string> getAllKeys() const;
    /*!
     * \brief 查找确认key是否存在
     * \return true:存在，false:不存在
     * */
    bool keyExists(const std::string &key);
    /*!
     * \brief get value via key
     * */
    template<class T>
    T getValue(const std::string &key) const noexcept;
/*!
 * \brief 设置key-value,如何key存在，value不同，则更新value
 * */
    template<class T>
    void setValue(const std::string &key, const T &value);
    /*!
     * \brief 根据key删除key-value
     * */
    void removeKey(const std::string& key);
    /*!
     * \brief 将此键值存储的键重置为其初始值
     * */
    void resetKey(const std::string& key);
    /*!
     * \brief 恢复此键值存储的单个键
     * */
    void recoverKey(const std::string& key);
    /*!
     * \brief 移除所有key-value
     * */
    void removeAllKeys();
    /*!
     * \brief 写入到磁盘，仅使用map时需要，若使用leveldb，则早已同步写入
     * */
    void syncToStorage();
    /*!
 * \brief 丢弃map中的更改，仅使用map时需要，若使用leveldb，则早已同步写入
 * */
    void discardPendingChanges();

private:
    KeyValueStorage();

private:
    /*!
     * \brief 底层使用leveldb作为kv数据库， 最简单的方法是，直接使用std::map作为底层数据结构
     * */
    std::unique_ptr<leveldb::DB> db_;
    leveldb::Options options_;
    leveldb::WriteOptions write_options_;
    leveldb::ReadOptions read_options_;
};

void createKeyValueStorage();
void resetKeyValueStorage();
/*!
 * \brief null
 * \err
PerErrc::kStorageNotFound
Returned if the passed InstanceSpecifier does not match any PersistencyKeyValueStorageInterfaceconfigured
 for this Executable.
PerErrc::kPhysicalStorageFailure
        Returned if access to the physical storage fails.
PerErrc::kIntegrityCorrupted
        Returned if stored data cannot be read because the structural integrity is corrupted.
PerErrc::kValidationFailed
        Returned if the validity of stored data cannot be ensured.
PerErrc::kEncryptionFailed
        Returned if the decryption of stored data fails.
PerErrc::kResourceBusy
        Returned if UpdatePersistency or ResetPersistency is currently being executed,
        or if RecoverKeyValue Storage or ResetKeyValueStorage is currently being executed
        for the same Key-Value Storage.
PerErrc::kOutOfStorageSpace
        Returned if the available physical storage space is insufficient for the values
        that are added/updated during an implicit update of the Key-Value Storage.
PerErrc::kQuotaExceeded
        Returned if the values that are added/updated during an implicit update of the Key-Value
        Storage would exceed the configured maximumAllowedSize.
PerErrc::kAuthenticationFailed
        Returned if checking the MAC of stored data fails.
 * */
void openKeyValueStorage();
void recoverKeyValueStorage();
std::uint64_t getCurrentKeyValueStorageSize();

}  // namespace per::kv


#endif //PER_KV_KEY_VALUE_STORAGE_H
