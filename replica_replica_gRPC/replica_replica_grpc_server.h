#ifndef BACKUP_PRIMARY_GRPC_SERVER_H
#define BACKUP_PRIMARY_GRPC_SERVER_H

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include "../lib_primary.h"
#include "../primary_backup_gRPC/primary_backup_grpc_client.h"
#include "backup_primary.grpc.pb.h"

using namespace std;

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::Status;

using backup_primary::BackupPrimarygRPC;
using backup_primary::PrimaryHeatbeatReply;
using backup_primary::PrimaryHeatbeatReq;
using backup_primary::RestoreDataReply;
using backup_primary::RestoreDataReq;

class BackupPrimarygRPCServiceImpl final : public BackupPrimarygRPC::Service {
 public:
  BackupPrimarygRPCServiceImpl(int mount_file_fd, LibPrimary* lib_primary,
                               PrimaryBackupgRPCClient* primary_backup_client);
  Status RestoreData(ServerContext* context,
                     ServerReader<RestoreDataReq>* reader,
                     RestoreDataReply* reply) override;
  Status PrimaryHeartbeat(ServerContext* context,
                          const PrimaryHeatbeatReq* request,
                          PrimaryHeatbeatReply* reply) override;

 private:
  int mount_file_fd_;
  LibPrimary* lib_primary_;
  PrimaryBackupgRPCClient* primary_backup_client_;
};

void RunServer(string serverAddress);

#endif