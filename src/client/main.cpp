#include <gflags/gflags.h>
#include <brpc/channel.h>
#include "kv_service.pb.h"

DEFINE_string(server, "127.0.0.1:8001", "Address of one server in the cluster");
DEFINE_string(command, "put", "Command to execute: 'put' or 'get'");
DEFINE_string(key, "", "The key for the operation");
DEFINE_string(value, "", "The value for the 'put' operation");

int main(int argc, char* argv[]) {
    google::ParseCommandLineFlags(&argc, &argv, true);

    // --- Setup brpc channel ---
    brpc::Channel channel;
    brpc::ChannelOptions options;
    options.protocol = "baidu_std";
    options.timeout_ms = 2000; // 2 seconds
    options.max_retry = 3;

    if (channel.Init(FLAGS_server.c_str(), &options) != 0) {
        LOG(ERROR) << "Failed to initialize channel to server";
        return -1;
    }

    // Create a stub to call the service
    kvstore::KvService_Stub stub(&channel);

    if (FLAGS_command == "put") {
        if (FLAGS_key.empty()) {
            LOG(ERROR) << "Key cannot be empty for a 'put' command.";
            return -1;
        }
        kvstore::KvCommand request;
        kvstore::PutResponse response;
        request.set_key(FLAGS_key);
        request.set_value(FLAGS_value);

        brpc::Controller cntl;
        stub.put(&cntl, &request, &response, nullptr);

        if (cntl.Failed()) {
            LOG(ERROR) << "RPC failed: " << cntl.ErrorText();
            return -1;
        }
        std::cout << "Put successful." << std::endl;

    } else if (FLAGS_command == "get") {
        if (FLAGS_key.empty()) {
            LOG(ERROR) << "Key cannot be empty for a 'get' command.";
            return -1;
        }
        kvstore::GetRequest request;
        kvstore::GetResponse response;
        request.set_key(FLAGS_key);

        brpc::Controller cntl;
        stub.get(&cntl, &request, &response, nullptr);

        if (cntl.Failed()) {
            LOG(ERROR) << "RPC failed: " << cntl.ErrorText();
            return -1;
        }

        if (response.success()) {
            std::cout << "Value: " << response.value() << std::endl;
        } else {
            std::cout << "Key not found." << std::endl;
        }
    } else {
        LOG(ERROR) << "Unknown command: " << FLAGS_command;
        return -1;
    }

    return 0;
}
