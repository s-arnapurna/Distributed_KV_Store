#pragma once

#include "storage/StorageEngine.h"
#include <braft/storage.h>
#include <braft/raft.h>
#include <atomic>

namespace kvstore {

class KeyValueStateMachine : public braft::StateMachine {
public:
    // Constructor now takes a StorageEngine pointer
    explicit KeyValueStateMachine(StorageEngine* storage_engine);
    ~KeyValueStateMachine() = default;

    // --- braft::StateMachine Interfaces ---
    void on_apply(braft::Iterator& iter) override;
    void on_snapshot_save(braft::SnapshotWriter* writer, braft::Closure* done) override;
    int on_snapshot_load(braft::SnapshotReader* reader) override;
    void on_leader_start(int64_t term) override;
    void on_leader_stop(const butil::Status& status) override;

    // --- Public API ---
    bool lookup(const std::string& key, std::string* value);
    bool is_leader() const {
        return _leader_term.load(std::memory_order_acquire) > 0;
    }

private:
    // No longer owns the DB, just has a pointer to the storage engine
    StorageEngine* _storage_engine;
    std::atomic<int64_t> _leader_term;
};

} // namespace kvstore
