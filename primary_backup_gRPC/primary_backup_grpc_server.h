#ifndef PRIMARY_BACKUP_GRPC_SERVER_H
#define PRIMARY_BACKUP_GRPC_SERVER_H

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include "../lib_primary.h"
#include "primary_backup.grpc.pb.h"

using namespace std;

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::Status;

using primary_backup::PrimaryBackupgRPC;
using primary_backup::SyncDataReply;
using primary_backup::SyncDataReq;

class PrimaryBackupgRPCServiceImpl final : public PrimaryBackupgRPC::Service {
 public:
  PrimaryBackupgRPCServiceImpl(int mount_file_fd, LibPrimary *lib_primary)
      : mount_file_fd_(mount_file_fd) {}
  Status SyncData(ServerContext *context, ServerReader<SyncDataReq> *reader,
                  SyncDataReply *reply) override;

 private:
  int mount_file_fd_;
};

#endif