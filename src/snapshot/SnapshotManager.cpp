#include "SnapshotManager.hpp"

SnapshotManager::SnapshotManager(BraftNode* raft, KeyValueStateMachine* fsm)
  : _raft(raft), _fsm(fsm) {}

void SnapshotManager::maybe_snapshot() {
  // braft triggers snapshot based on configured intervals
}
