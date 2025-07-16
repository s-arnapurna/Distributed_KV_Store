#pragma once
// Optional manager to coordinate snapshot calls
#include "consensus/BraftNode.hpp"
#include "state/KeyValueStateMachine.hpp"

class SnapshotManager {
public:
  SnapshotManager(BraftNode* raft, KeyValueStateMachine* fsm);
  void maybe_snapshot();
private:
  BraftNode* _raft;
  KeyValueStateMachine* _fsm;
};
