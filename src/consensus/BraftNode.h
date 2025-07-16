#pragma once

#include "state/KeyValueStateMachine.h"
#include "storage/StorageEngine.h"
#include <braft/raft.h>

namespace kvstore {

class BraftNode {
public:
    BraftNode(const std::string& group_id, const braft::Configuration& initial_conf, const std::string& data_path, const butil::EndPoint& listen_addr);
    ~BraftNode();

    int start();
    void shutdown();
    void apply(const KvCommand& cmd, google::protobuf::Closure* done);
    KeyValueStateMachine* get_fsm() { return _fsm; }
    bool is_leader();
    braft::PeerId leader_id() { return _node ? _node->leader_id() : braft::PeerId(); }

private:
    std::string _group_id;
    std::string _data_path;
    butil::EndPoint _listen_addr;
    braft::Configuration _initial_conf;
    
    braft::Node* _node;
    KeyValueStateMachine* _fsm;
    StorageEngine* _storage_engine;
};

} // namespace kvstore
