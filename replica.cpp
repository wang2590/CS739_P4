#include <fcntl.h>
#include <unistd.h>

#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>

#include "client_replica_gRPC/client_replica_grpc_server.h"
#include "common.h"
#include "replica_replica_gRPC/replica_replica_grpc_client.h"
#include "replica_replica_gRPC/replica_replica_grpc_server.h"

using nlohmann::json;

void usage(char *argv[]) { printf("usage: %s -c config_file [-h]\n", argv[0]); }

int main(int argc, char *argv[]) {
  extern char *optarg;
  int opt;
  std::string config_file;
  while ((opt = getopt(argc, argv, "c:h")) != -1) {
    switch (opt) {
      case 'c':
        config_file = optarg;
        break;
      case 'h':
        usage(argv);
        exit(0);
      case '?':
        usage(argv);
        exit(1);
    }
  }
  if (config_file == "") {
    usage(argv);
    exit(1);
  }

  std::ifstream config_file_stream(config_file);
  json config = json::parse(config_file_stream);

  std::string listen_addr_port = config["ip_port"];

  // replica state
  ReplicaState state;
  state.replica_id = config["replica_id"];
  state.private_key = CreateRsaWithFilename(config["private_key_path"], false);
  for (auto &replica_conf : config["replicas"]) {
    state.replicas_public_keys.push_back(
        CreateRsaWithFilename(replica_conf["public_key_path"], true));
  }

  // open mount file
  state.mount_file_fd = get_mount_file();

  // grpc client
  for (auto &replica_conf : config["replicas"]) {
    if (state.replica_clients.size() == config["replica_id"]) {
      state.replica_clients.push_back(nullptr);
    } else {
      auto client = std::make_unique<ReplicaReplicaGrpcClient>(
          grpc::CreateChannel(replica_conf["ip_port"],
                              grpc::InsecureChannelCredentials()),
          &state);
      state.replica_clients.push_back(std::move(client));
    }
  }

  // setup default state
  state.primary = 0;
  state.view = 0;
  state.f = (state.replica_clients.size() - 1) / 3;

  // grpc server
  ReplicaReplicaGrpcServiceImpl service1(&state);
  ClientReplicaGrpcServiceImpl service2(&state);
  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();
  ServerBuilder builder;
  builder.AddListeningPort(listen_addr_port, grpc::InsecureServerCredentials());
  builder.RegisterService(&service1);
  builder.RegisterService(&service2);
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << listen_addr_port << std::endl;
  server->Wait();

  // close file
  if (close(state.mount_file_fd) < 0) {
    perror("close");
    exit(1);
  }
}