#include <gflags/gflags.h>
#include <brpc/server.h>
#include <braft/raft.h>
#include "api/KVServer.h"
#include "consensus/BraftNode.h"

// --- Command line flags ---
DEFINE_string(conf, "", "Configuration of the raft group, e.g., 127.0.0.1:8100,127.0.0.1:8101");
DEFINE_string(data_path, "./data", "Path to store raft data");
DEFINE_string(group_id, "KvStore", "Name of the raft group");
DEFINE_int32(port, 8100, "Port of this server");

int main(int argc, char* argv[]) {
    google::ParseCommandLineFlags(&argc, &argv, true);

    // --- Setup brpc server ---
    brpc::Server server;
    
    // Parse the listen address and initial configuration
    butil::EndPoint listen_addr(butil::my_ip(), FLAGS_port);
    braft::Configuration initial_conf;
    if (FLAGS_conf.empty()) {
        LOG(ERROR) << "Configuration flag --conf cannot be empty.";
        return -1;
    }
    if (initial_conf.parse_from(FLAGS_conf) != 0) {
        LOG(ERROR) << "Failed to parse initial configuration: " << FLAGS_conf;
        return -1;
    }

    // --- Initialize Raft Node ---
    kvstore::BraftNode node(FLAGS_group_id, initial_conf, FLAGS_data_path, listen_addr);
    if (node.start() != 0) {
        LOG(ERROR) << "Raft node failed to start";
        return -1;
    }

    // --- Add our KV service to the brpc server ---
    kvstore::KvServiceImpl kv_service(&node);
    if (server.AddService(&kv_service, brpc::SERVER_DOESNT_OWN_SERVICE) != 0) {
        LOG(ERROR) << "Failed to add KvService";
        return -1;
    }

    // It's necessary to start braft's RPC service
    if (braft::add_service(&server, listen_addr) != 0) {
        LOG(ERROR) << "Failed to add braft service";
        return -1;
    }

    // --- Start the brpc server ---
    if (server.Start(listen_addr, nullptr) != 0) {
        LOG(ERROR) << "Failed to start brpc server";
        return -1;
    }
    LOG(INFO) << "KV Server listening on " << listen_addr;

    // Wait until 'CTRL-C' is pressed.
    while (!brpc::IsAskedToQuit()) {
        sleep(1);
    }

    LOG(INFO) << "KV Server is shutting down";
    node.shutdown();
    server.Stop(0);
    server.Join();

    return 0;
}
