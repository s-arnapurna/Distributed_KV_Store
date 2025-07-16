#include "StorageEngine.h"
#include <butil/logging.h>

namespace kvstore {

RocksDBStorageEngine::RocksDBStorageEngine(const std::string& db_path)
    : _db(nullptr), _db_path(db_path) {}

RocksDBStorageEngine::~RocksDBStorageEngine() {
    shutdown();
}

bool RocksDBStorageEngine::start() {
    rocksdb::Options options;
    options.create_if_missing = true;
    rocksdb::Status status = rocksdb::DB::Open(options, _db_path, &_db);
    if (!status.ok()) {
        LOG(ERROR) << "Failed to open RocksDB at " << _db_path << ": " << status.ToString();
        return false;
    }
    LOG(INFO) << "RocksDB started at " << _db_path;
    return true;
}

void RocksDBStorageEngine::shutdown() {
    if (_db) {
        delete _db;
        _db = nullptr;
    }
}

bool RocksDBStorageEngine::put(const std::string& key, const std::string& value) {
    rocksdb::Status status = _db->Put(rocksdb::WriteOptions(), key, value);
    if (!status.ok()) {
        LOG(ERROR) << "Failed to write to RocksDB: " << status.ToString();
    }
    return status.ok();
}

bool RocksDBStorageEngine::get(const std::string& key, std::string* value) {
    rocksdb::Status status = _db->Get(rocksdb::ReadOptions(), key, value);
    return status.ok();
}

rocksdb::Iterator* RocksDBStorageEngine::new_iterator() {
    return _db->NewIterator(rocksdb::ReadOptions());
}

bool RocksDBStorageEngine::destroy_and_reopen() {
    shutdown();
    rocksdb::Status status = rocksdb::DestroyDB(_db_path, rocksdb::Options());
    if (!status.ok()) {
        LOG(ERROR) << "Failed to destroy RocksDB: " << status.ToString();
    }
    return start();
}

} // namespace kvstore
