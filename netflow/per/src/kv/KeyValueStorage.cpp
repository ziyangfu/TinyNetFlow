#include "kv/KeyValueStorage.h"
#include <iostream>
#include <sstream>
#include <stdexcept>
//#include "spdlog/spdlog.h"

namespace per::kv {

/*!
 * \private
 * */
KeyValueStorage::KeyValueStorage(Configration& configs)
    : configs_(configs),
      snapshot_(nullptr)
{
    if (configs_.keyValueStorageType_ == KeyValueStorageType::kLeveldb) {
        options_.create_if_missing = true; /** 避免数据库已存在时引发错误 */
    }
    init();
}

void KeyValueStorage::init() {
    if (configs_.keyValueStorageType_ == KeyValueStorageType::kLeveldb) {
        auto dbp = db_.get();
        leveldb::Status status = leveldb::DB::Open(options_, configs_.dbPath_, &dbp);
        if (!status.ok()) {
            throw std::runtime_error("Unable to open/create db: " + status.ToString());
        }
    }
    else if (configs_.keyValueStorageType_ == KeyValueStorageType::kMap) {
        throw std::runtime_error("Map storage not implemented");
    }

}

std::vector<std::string> KeyValueStorage::getAllKeys() const {
    std::vector<std::string> keys;
    if (configs_.keyValueStorageType_ == KeyValueStorageType::kLeveldb) {

        std::unique_ptr<leveldb::Iterator> it(db_->NewIterator(readOptions_));
        for (it->SeekToFirst(); it->Valid(); it->Next()) {
            keys.push_back(it->key().ToString());
        }
        if (!it->status().ok()) {
            throw std::runtime_error("Iterator error: " + it->status().ToString());
        }
    }
    else if (configs_.keyValueStorageType_ == KeyValueStorageType::kMap) {
        throw std::runtime_error("Map storage not implemented");
    }
    return keys;
}

bool KeyValueStorage::keyExists(const std::string &key) {
    std::string value;
    bool result {false};
    if (configs_.keyValueStorageType_ == KeyValueStorageType::kLeveldb) {
        result = db_->Get(readOptions_, key, &value).ok();
    }
    else if (configs_.keyValueStorageType_ == KeyValueStorageType::kMap) {
        throw std::runtime_error("Map storage not implemented");
    }
    return result;
}

template<class T>
T KeyValueStorage::getValue(const std::string &key) const noexcept {
    T value{};
    if (configs_.keyValueStorageType_ == KeyValueStorageType::kLeveldb) {
        std::string value_str;
        if (db_->Get(readOptions_, key, &value_str).ok()) {
            std::istringstream iss(value_str);
            iss >> value;
        }
    }
    return value;
}

template<class T>
void KeyValueStorage::setValue(const std::string &key, const T &value) {
    std::string value_str;
    leveldb::Status status = db_->Put(writeOptions_, key, value_str);
}

void KeyValueStorage::removeKey(const std::string& key) {
    leveldb::Status status = db_->Delete(writeOptions_, key);
    if (!status.ok()) {
        throw std::runtime_error("Failed to remove key: " + status.ToString());
    }
}

void KeyValueStorage::resetKey(const std::string& key) {
    // Assuming reset means setting the value to a default value of T
//    T default_value{};
//    setValue(key, default_value);
}

void KeyValueStorage::recoverKey(const std::string& key) {
    // Assuming recover means restoring the key to its last committed state
    // This is a no-op in LevelDB as it doesn't support undo operations
    // Implementing a proper recovery mechanism would require additional logic
}

void KeyValueStorage::removeAllKeys() {
    for (const auto& key : getAllKeys()) {
        removeKey(key);
    }
}

/*!
 * \brief  使用leveldb时，仅记录一次快照 snapshot
 * */
void KeyValueStorage::syncToStorage() {
    if (configs_.keyValueStorageType_ == KeyValueStorageType::kLeveldb) {
        if (snapshot_ != nullptr) {
            //delete snapshot_;
        }
        auto snapshottt_ = db_->GetSnapshot();
    }
    if (configs_.keyValueStorageType_ == KeyValueStorageType::kMap) {
     /** 写入磁盘 */
    }
}

/*!
 * \brief  使用leveldb时，恢复到上一次snapshot
 * */
void KeyValueStorage::discardPendingChanges() {
    if (configs_.keyValueStorageType_ == KeyValueStorageType::kLeveldb) {
        if (snapshot_ != nullptr) {
            db_->ReleaseSnapshot(snapshot_);
            snapshot_ = nullptr;
        }
    }
}

void KeyValueStorage::destroyKeyValueStorage() {

}

bool KeyValueStorage::setSyncWriteAccess(bool syncWriteAccess) {
    bool result {false};
    if (configs_.accessMode_ == AccessMode::kRead) {
        throw std::runtime_error("Cannot set sync write access in read-only mode");
    }
    if (configs_.keyValueStorageType_ == KeyValueStorageType::kLeveldb) {
        writeOptions_.sync = syncWriteAccess;
        result = true;
    }
    return result;

}

void createKeyValueStorage() {
    // Implementation for creating a new storage instance
}

void resetKeyValueStorage() {
    // Implementation for resetting the storage
}

void openKeyValueStorage() {
    // Implementation for opening an existing storage
}

void recoverKeyValueStorage() {
    // Implementation for recovering the storage
}

std::uint64_t getCurrentKeyValueStorageSize() {
    // Implementation for getting the current size of the storage
    return 0; // Placeholder
}

}  // namespace per::kv

