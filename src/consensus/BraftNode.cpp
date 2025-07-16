#include "BraftNode.h"
#include <butil/logging.h>

namespace kvstore {

BraftNode::BraftNode(const std::string& group_id, const braft::Configuration& initial_conf, const std::string& data_path, const butil::EndPoint& listen_addr)
    : _group_id(group_id),
      _data_path(data_path),
      _listen_addr(listen_addr),
      _initial_conf(initial_conf),
      _node(nullptr) {
    // BraftNode now creates both the storage engine and the state machine
    _storage_engine = new RocksDBStorageEngine(_data_path + "/db");
    _fsm = new KeyValueStateMachine(_storage_engine);
}

BraftNode::~BraftNode() {
    shutdown();
    delete _fsm;
    delete _storage_engine;
    delete _node;
}

int BraftNode::start() {
    // Start the storage engine first
    if (!_storage_engine->start()) {
        LOG(ERROR) << "StorageEngine failed to start";
        return -1;
    }

    braft::NodeOptions node_options;
    node_options.fsm = _fsm;
    node_options.initial_conf = _initial_conf;
    node_options.snapshot_interval_s = 30; // Take a snapshot every 30 seconds
    node_options.log_uri = _data_path + "/log";
    node_options.raft_meta_uri = _data_path + "/raft_meta";
    node_options.snapshot_uri = _data_path + "/snapshot";
    node_options.disable_cli = false;

    _node = new braft::Node(_group_id, braft::PeerId(_listen_addr));
    
    if (_node->init(node_options) != 0) {
        LOG(ERROR) << "Failed to init raft node";
        delete _node;
        _node = nullptr;
        return -1;
    }
    LOG(INFO) << "Raft node started.";
    return 0;
}

void BraftNode::shutdown() {
    if (_node) {
        _node->shutdown(nullptr);
        _node->join();
    }
    if (_storage_engine) {
        _storage_engine->shutdown();
    }
}

void BraftNode::apply(const KvCommand& cmd, google::protobuf::Closure* done) {
    butil::IOBuf data;
    butil::IOBufAsZeroCopyOutputStream wrapper(&data);
    if (!cmd.SerializeToZeroCopyStream(&wrapper)) {
        LOG(ERROR) << "Failed to serialize KvCommand";
        // In a real application, you would run the done closure with an error status
        return;
    }

    braft::Task task;
    task.data = &data;
    task.done = done;
    _node->apply(task);
}

bool BraftNode::is_leader() {
    return _node && _node->is_leader();
}

} // namespace kvstore
