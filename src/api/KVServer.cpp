#include "KVServer.h"
#include <brpc/controller.h>
#include <brpc/closure_guard.h>
#include <braft/raft.h>

namespace kvstore {

void KvServiceImpl::get(google::protobuf::RpcController* controller,
                        const GetRequest* request,
                        GetResponse* response,
                        google::protobuf::Closure* done) {
    brpc::ClosureGuard done_guard(done);

    std::string value;
    bool found = _node->get_fsm()->lookup(request->key(), &value);

    response->set_success(found);
    if (found) {
        response->set_value(value);
    }
}

void KvServiceImpl::put(google::protobuf::RpcController* controller,
                        const KvCommand* request,
                        PutResponse* response,
                        google::protobuf::Closure* done) {
    brpc::Controller* cntl = static_cast<brpc::Controller*>(controller);
    brpc::ClosureGuard done_guard(done);

    if (!_node->is_leader()) {
        braft::PeerId leader = _node->leader_id();
        if (!leader.is_empty()) {
            cntl->SetFailed(brpc::EREQUEST, "Not leader, redirecting to %s", leader.to_string().c_str());
        } else {
            cntl->SetFailed(brpc::EREQUEST, "Not leader and no leader available");
        }
        return;
    }

    // Propose the command to the braft group.
    // The `done` closure will be called by braft after this log entry is committed.
    // We create a new closure to wrap the original one and set the response.
    auto* braft_done = new google::protobuf::Closure([response, done_guard_capture = done_guard.release()]() {
        response->set_success(true);
        brpc::ClosureGuard done_guard(done_guard_capture); // This will run the original done
    });
    
    _node->apply(*request, braft_done);
}

} // namespace kvstore
