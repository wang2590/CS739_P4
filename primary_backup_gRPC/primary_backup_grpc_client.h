#ifndef PRIMARY_BACKUP_GRPC_CLIENT_H
#define PRIMARY_BACKUP_GRPC_CLIENT_H

#include <grpcpp/grpcpp.h>

#include "primary_backup.grpc.pb.h"

using namespace std;
using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using primary_backup::PrimaryBackupgRPC;
using primary_backup::SyncDataReply;
using primary_backup::SyncDataReq;

class PrimaryBackupgRPCClient {
 private:
  std::unique_ptr<PrimaryBackupgRPC::Stub> stub_;
  int file_d;
  string read(const int offset);

 public:
  PrimaryBackupgRPCClient(std::shared_ptr<Channel> channel,
                          const int& mount_file_fd);
  int clientSyncData(const vector<int>& offset_v);
};

#endif

/*
How to use this class:
default largest gRPC size = 4MB

const std::string target_str = "128.105.145.95:75247";
PrimaryBackupgRPCClient * grpcClient = new
PrimaryBackupgRPCClient(grpc::CreateChannel(target_str,
grpc::InsecureChannelCredentials(), ch_args));

*/