#include "SystemCoordinator.hpp"

SystemCoordinator::SystemCoordinator(BraftNode* raft) : _raft(raft) {}

void SystemCoordinator::monitor() {
  // TODO: dynamic membership via braft add_peer/remove_peer
}
