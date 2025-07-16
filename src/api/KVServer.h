#pragma once

#include "consensus/BraftNode.h"
#include "kv_service.pb.h"

namespace kvstore {

class KvServiceImpl : public KvService {
public:
    explicit KvServiceImpl(BraftNode* node) : _node(node) {}

    void get(google::protobuf::RpcController* controller,
             const GetRequest* request,
             GetResponse* response,
             google::protobuf::Closure* done) override;

    void put(google::protobuf::RpcController* controller,
             const KvCommand* request,
             PutResponse* response,
             google::protobuf::Closure* done) override;

private:
    BraftNode* _node;
};

} // namespace kvstore
