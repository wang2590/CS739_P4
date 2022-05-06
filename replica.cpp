#include <fcntl.h>
#include <unistd.h>

#include <cstdio>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>

#include "client_replica_gRPC/client_replica_grpc_server.h"
#include "common.h"
#include "replica_replica_gRPC/replica_replica_grpc_client.h"
#include "replica_replica_gRPC/replica_replica_grpc_server.h"

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

  const std::string priv_key_path = config["private_key_path"];
  std::string listen_addr_port = config["ip_port"];

  // open mount file
  int mount_file_fd = get_mount_file();

  // grpc client
  grpc::ChannelArguments args;
  std::vector<std::unique_ptr<ReplicaReplicaGrpcClient>> replica_clients;
  for (auto &replica_conf : config["replicas"]) {
    if (replica_clients.size() == config["replica_id"]) {
      replica_clients.push_back(nullptr);
    } else {
      auto client = std::make_unique<ReplicaReplicaGrpcClient>(
          grpc::CreateChannel(replica_conf["ip_port"],
                              grpc::InsecureChannelCredentials()),
          mount_file_fd);
      replica_clients.push_back(std::move(client));
    }
  }

  // grpc server
  ReplicaReplicaGrpcServer service1(mount_file_fd, primary_backup_client.get());
  ClientReplicaGrpcServiceImpl service2(mount_file_fd);
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
  if (close(mount_file_fd) < 0) {
    perror("close");
    exit(1);
  }
}