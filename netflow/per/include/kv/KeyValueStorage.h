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

class KeyValueStorage {
public:
    KeyValueStorage();
    KeyValueStorage(const KeyValueStorage &) = delete;
    KeyValueStorage& operator=(const KeyValueStorage &) = delete;

    KeyValueStorage(KeyValueStorage &&) = default;
    KeyValueStorage& operator=(KeyValueStorage &&) = default;

    void init(const std::string& db_path);

    std::vector<std::string> getAllKeys();
    bool keyExists(const std::string &key);

    template<class T>
    T getValue(const std::string &key, T &value) const noexcept;

    template<class T>
    void setValue(const std::string &key, const T &value);

    void removeKey(const std::string& key);
    void resetKey(const std::string& key);
    void recoverKey(const std::string& key);

    void removeAllKeys();
    void syncToStorage();
    void discardPendingChanges();

private:
    std::unique_ptr<leveldb::DB> db_;
    leveldb::Options options_;
    leveldb::WriteOptions write_options_;
    leveldb::ReadOptions read_options_;
};

void createKeyValueStorage();
void resetKeyValueStorage();
void openKeyValueStorage();
void recoverKeyValueStorage();
std::uint64_t getCurrentKeyValueStorageSize();


#if 0
namespace per::kv {

class KeyValueStorage {
public:
    KeyValueStorage();
    KeyValueStorage(const KeyValueStorage &) = delete;
    KeyValueStorage& operator=(const KeyValueStorage &) = delete;

    KeyValueStorage(KeyValueStorage &&) = default;
    KeyValueStorage& operator=(KeyValueStorage &&) = default;

    void init();

    void getAllKeys();
    bool keyExists(const std::string &key);




  ~KeyValueStorage() = default;
  /*!
   * \brief get value via key
   * */
    template<class T>
    T getValue(const std::string &key, T &value) const noexcept;

  template<class T>
  void setValue(const std::string &key, const T &value);

    /*!
     * \brief 从此键值存储中删除一个键和关联的值
     * \details RemoveKey 可能会因另一个线程对 RemoveAllKeys、SyncToStorage 的持续调用而延迟
     * */
  void removeKey(const std::string& key);
    /*!
     * \brief 将此键值存储的键重置为其初始值
     * */
  void resetKey(const std::string& key);



  /*!
   * \brief 恢复此键值存储的单个键
   * */
  void recoverKey(std::string& key)

  void removeAllKeys();

/*!
 * \brief 临时修改kv存储操作，只有调用syncToStorage才会落盘
 * */
  void syncToStorage();

  void discardPendingChanges();


};

void createKeyValueStorage();
void resetKeyValueStorage();
void openKeyValueStorage();
void RecoverKeyValueStorage();
std::uint64_t  GetCurrentKeyValueStorageSize();

}  // namespace per::kv
#endif

#endif //PER_KV_KEY_VALUE_STORAGE_H
