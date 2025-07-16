#include "KeyValueStateMachine.h"
#include "kv_service.pb.h"
#include <butil/logging.h>
#include <braft/util.h>
#include <sstream>

namespace kvstore {

// Constructor updated
KeyValueStateMachine::KeyValueStateMachine(StorageEngine* storage_engine)
    : _storage_engine(storage_engine), _leader_term(-1) {}

void KeyValueStateMachine::on_apply(braft::Iterator& iter) {
    for (; iter.valid(); iter.next()) {
        braft::AsyncClosureGuard closure_guard(iter.done());

        butil::IOBuf data = iter.data();
        KvCommand cmd;
        if (!cmd.ParseFromArray(data.data(), data.size())) {
            LOG(ERROR) << "Failed to parse KvCommand from raft log";
            continue;
        }

        // Delegate write to the storage engine
        _storage_engine->put(cmd.key(), cmd.value());
    }
}

bool KeyValueStateMachine::lookup(const std::string& key, std::string* value) {
    // Delegate read to the storage engine
    return _storage_engine->get(key, value);
}

void KeyValueStateMachine::on_snapshot_save(braft::SnapshotWriter* writer, braft::Closure* done) {
    braft::AsyncClosureGuard done_guard(done);
    LOG(INFO) << "Saving snapshot...";

    // Get iterator from the storage engine
    rocksdb::Iterator* it = _storage_engine->new_iterator();
    
    // In a real system, you'd add checksums and more robust serialization.
    for (it->SeekToFirst(); it->Valid(); it->Next()) {
        std::string kv_pair = it->key().ToString() + ":" + it->value().ToString() + "\n";
        butil::IOBuf data;
        data.append(kv_pair);
        // A real implementation would likely use a more robust file format.
        writer->add_file("data.txt", &data);
    }

    delete it;
    LOG(INFO) << "Snapshot saved.";
}

int KeyValueStateMachine::on_snapshot_load(braft::SnapshotReader* reader) {
    LOG(INFO) << "Loading snapshot...";
    
    // Delegate clearing the DB to the storage engine
    if (!_storage_engine->destroy_and_reopen()) {
        LOG(ERROR) << "Failed to clear DB for snapshot load";
        return -1;
    }

    std::vector<std::string> files;
    reader->list_files(&files);
    for (const auto& file : files) {
        if (file == "data.txt") {
            butil::IOBuf data;
            reader->get_file(file, &data);
            std::string content = data.to_string();
            std::stringstream ss(content);
            std::string line;
            while(std::getline(ss, line, '\n')) {
                size_t colon_pos = line.find(':');
                if (colon_pos != std::string::npos) {
                    std::string key = line.substr(0, colon_pos);
                    std::string value = line.substr(colon_pos + 1);
                    // Delegate write to the storage engine
                    _storage_engine->put(key, value);
                }
            }
        }
    }
    LOG(INFO) << "Snapshot loaded.";
    return 0;
}

void KeyValueStateMachine::on_leader_start(int64_t term) {
    _leader_term.store(term, std::memory_order_release);
    LOG(INFO) << "Node becomes leader on term " << term;
}

void KeyValueStateMachine::on_leader_stop(const butil::Status& status) {
    _leader_term.store(-1, std::memory_order_release);
    LOG(INFO) << "Node steps down from leader: " << status.error_str();
}

} // namespace kvstore
