#include "PeerTransport.hpp"
#include "consensus/BraftNode.hpp"

PeerTransport::PeerTransport(BraftNode* node) : _node(node) {}

void PeerTransport::start(const std::string& ip_port) {
  // TODO: register Braft and service handlers with brpc server
  _server.Start(ip_port.c_str(), nullptr);
  _server.RunUntilAskedToQuit();
}
