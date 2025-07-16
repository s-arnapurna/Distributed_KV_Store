#pragma once

#include <rocksdb/db.h>
#include <string>

namespace kvstore {

// Abstract interface for the underlying storage.
// This allows swapping out RocksDB for another engine in the future.
class StorageEngine {
public:
    virtual ~StorageEngine() = default;

    // Starts the storage engine.
    virtual bool start() = 0;

    // Shuts down the storage engine.
    virtual void shutdown() = 0;

    // Puts a key-value pair into storage.
    virtual bool put(const std::string& key, const std::string& value) = 0;

    // Gets a value for a given key.
    virtual bool get(const std::string& key, std::string* value) = 0;

    // Returns a new iterator for scanning the database.
    // The caller is responsible for deleting the iterator.
    virtual rocksdb::Iterator* new_iterator() = 0;

    // Clears all data and re-initializes the database.
    // Used for loading a snapshot.
    virtual bool destroy_and_reopen() = 0;
};

// RocksDB implementation of the StorageEngine interface.
class RocksDBStorageEngine : public StorageEngine {
public:
    explicit RocksDBStorageEngine(const std::string& db_path);
    ~RocksDBStorageEngine() override;

    bool start() override;
    void shutdown() override;
    bool put(const std::string& key, const std::string& value) override;
    bool get(const std::string& key, std::string* value) override;
    rocksdb::Iterator* new_iterator() override;
    bool destroy_and_reopen() override;

private:
    rocksdb::DB* _db;
    std::string _db_path;
};

} // namespace kvstore
