#pragma once
#include <string>
#include <vector>

class BraftNode;

class SystemCoordinator {
public:
  SystemCoordinator(BraftNode* raft);
  void monitor();
private:
  BraftNode* _raft;
};
