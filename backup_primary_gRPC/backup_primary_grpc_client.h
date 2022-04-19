#ifndef BACKUP_PRIMARY_GRPC_CLIENT_H
#define BACKUP_PRIMARY_GRPC_CLIENT_H

#include <grpcpp/grpcpp.h>

#include "backup_primary.grpc.pb.h"

using namespace std;
using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using backup_primary::BackupPrimarygRPC;
using backup_primary::PrimaryHeatbeatReply;
using backup_primary::PrimaryHeatbeatReq;
using backup_primary::RestoreDataReply;
using backup_primary::RestoreDataReq;

class BackupPrimarygRPCClient {
 private:
  std::unique_ptr<BackupPrimarygRPC::Stub> stub_;
  int file_fd;
  string read(const int offset);

 public:
  BackupPrimarygRPCClient(std::shared_ptr<Channel> channel,
                          const int& mount_file_fd);
  int clientRestoreData(const vector<int>& offset_v);
  bool clientPrimaryHeartbeat();
};

#endif

/*
How to use this class:
default largest gRPC size = 4MB

const std::string target_str = "128.105.145.95:75247";
BackupPrimarygRPCClient * grpcClient = new
BackupPrimarygRPCClient(grpc::CreateChannel(target_str,
grpc::InsecureChannelCredentials(), ch_args));

*/