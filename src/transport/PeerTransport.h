#pragma once
// Using brpc for network transport
#include <brpc/server.h>

class BraftNode;

class PeerTransport {
public:
  PeerTransport(BraftNode* node);
  void start(const std::string& ip_port);
private:
  BraftNode* _node;
  brpc::Server _server;
};
