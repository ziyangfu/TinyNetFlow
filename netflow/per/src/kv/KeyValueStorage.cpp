#include "kv/KeyValueStorage.h"

#include <iostream>
#include <sstream>
#include <stdexcept>

namespace per::kv {

KeyValueStorage::KeyValueStorage() {
    options_.create_if_missing = true;
}

void KeyValueStorage::init(const std::string& db_path) {
    leveldb::Status status = leveldb::DB::Open(options_, db_path, &db_);
    if (!status.ok()) {
        throw std::runtime_error("Unable to open/create db: " + status.ToString());
    }
}

std::vector<std::string> KeyValueStorage::getAllKeys() {
    std::vector<std::string> keys;
    std::unique_ptr<leveldb::Iterator> it(db_->NewIterator(read_options_));
    for (it->SeekToFirst(); it->Valid(); it->Next()) {
        keys.push_back(it->key().ToString());
    }
    if (!it->status().ok()) {
        throw std::runtime_error("Iterator error: " + it->status().ToString());
    }
    return keys;
}

bool KeyValueStorage::keyExists(const std::string &key) {
    std::string value;
    return db_->Get(read_options_, key, &value).ok();
}

template<class T>
T KeyValueStorage::getValue(const std::string &key, T &value) const noexcept {
    std::string value_str;
    if (db_->Get(read_options_, key, &value_str).ok()) {
        std::istringstream iss(value_str);
        iss >> value;
    }
    return value;
}

template<class T>
void KeyValueStorage::setValue(const std::string &key, const T &value) {
    std::ostringstream oss;
    oss << value;
    leveldb::Status status = db_->Put(write_options_, key, oss.str());
    if (!status.ok()) {
        throw std::runtime_error("Failed to set value: " + status.ToString());
    }
}

void KeyValueStorage::removeKey(const std::string& key) {
    leveldb::Status status = db_->Delete(write_options_, key);
    if (!status.ok()) {
        throw std::runtime_error("Failed to remove key: " + status.ToString());
    }
}

void KeyValueStorage::resetKey(const std::string& key) {
    // Assuming reset means setting the value to a default value of T
    T default_value{};
    setValue(key, default_value);
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

void KeyValueStorage::syncToStorage() {
    // LevelDB writes are synchronous by default, so this is a no-op
}

void KeyValueStorage::discardPendingChanges() {
    // LevelDB doesn't have a concept of pending changes that can be discarded
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

