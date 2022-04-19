#include <fcntl.h>
#include <unistd.h>

#include <cstdio>
#include <iostream>
#include <memory>
#include <string>

#include "backup_primary_gRPC/backup_primary_grpc_server.h"
#include "client_server_gRPC/client_server_grpc_server.h"
#include "common.h"
#include "primary_backup_gRPC/primary_backup_grpc_client.h"

void usage(char *argv[]) {
  printf("usage: %s -l listen_addr_port -t backup_ip_port [-h]\n", argv[0]);
}

int main(int argc, char *argv[]) {
  extern char *optarg;
  int opt;
  std::string listen_addr_port;
  std::string backup_ip_port;
  while ((opt = getopt(argc, argv, "l:t:h")) != -1) {
    switch (opt) {
      case 'l':
        listen_addr_port = optarg;
        break;
      case 't':
        backup_ip_port = optarg;
        break;
      case 'h':
        usage(argv);
        exit(0);
      case '?':
        usage(argv);
        exit(1);
    }
  }
  if (listen_addr_port == "" || backup_ip_port == "") {
    usage(argv);
    exit(1);
  }

  // open mount file
  int mount_file_fd = get_mount_file();

  // grpc client
  grpc::ChannelArguments args;
  auto primary_backup_client = std::make_unique<PrimaryBackupgRPCClient>(
      grpc::CreateChannel(backup_ip_port, grpc::InsecureChannelCredentials()),
      mount_file_fd);

  // lib_primary
  auto lib_primary = std::make_unique<LibPrimary>(true, mount_file_fd,
                                                  primary_backup_client.get());

  // grpc server
  BackupPrimarygRPCServiceImpl service1(mount_file_fd, lib_primary.get(),
                                        primary_backup_client.get());
  ClientServergRPCServiceImpl service2(mount_file_fd, lib_primary.get());
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