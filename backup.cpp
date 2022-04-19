#include <fcntl.h>
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include "backup_primary_gRPC/backup_primary_grpc_client.h"
#include "client_server_gRPC/client_backup_grpc_server.h"
#include "common.h"
#include "primary_backup_gRPC/primary_backup_grpc_server.h"

std::atomic_bool primary_alive = true;
std::unique_ptr<LibPrimary> lib_primary;

std::string listen_addr_port;
std::string primary_ip_port;

int mount_file_fd;

void usage(char *argv[]) {
  printf("usage: %s -l listen_addr_port -t primary_ip_port [-h]\n", argv[0]);
}

void heartbeat_thread() {
  grpc::ChannelArguments args;
  auto backup_primary_client = std::make_unique<BackupPrimarygRPCClient>(
      grpc::CreateChannel(primary_ip_port, grpc::InsecureChannelCredentials()),
      mount_file_fd);

  while (true) {
    if (backup_primary_client->clientPrimaryHeartbeat()) {
      if (primary_alive == false) {
        primary_alive = true;
        std::vector<int> restore_queue = lib_primary->GetRestoreQueue();
        backup_primary_client->clientRestoreData(restore_queue);
      }
    } else {
      primary_alive = false;
    }

    std::this_thread::sleep_for(std::chrono::seconds(kHeartbeatSecs));
  }
}

int main(int argc, char *argv[]) {
  extern char *optarg;
  int opt;
  while ((opt = getopt(argc, argv, "l:t:h")) != -1) {
    switch (opt) {
      case 'l':
        listen_addr_port = optarg;
        break;
      case 't':
        primary_ip_port = optarg;
        break;
      case 'h':
        usage(argv);
        exit(0);
      case '?':
        usage(argv);
        exit(1);
    }
  }
  if (listen_addr_port == "" || primary_ip_port == "") {
    usage(argv);
    exit(1);
  }

  // open mount file
  mount_file_fd = get_mount_file();

  // lib_primary
  lib_primary = std::make_unique<LibPrimary>(false, mount_file_fd, nullptr);

  // grpc client
  std::thread grpc_client_thread(heartbeat_thread);

  // grpc server
  PrimaryBackupgRPCServiceImpl service1(mount_file_fd, lib_primary.get());
  ClientBackupgRPCServiceImpl service2(mount_file_fd, lib_primary.get());
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