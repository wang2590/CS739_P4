#ifndef BACKUP_PRIMARY_GRPC_CLIENT_H
#define BACKUP_PRIMARY_GRPC_CLIENT_H

#include <grpcpp/grpcpp.h>

#include <memory>

#include "replica_replica.grpc.pb.h"

using namespace std;

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using replica_replica::ReplicaReplicaGrpc;

class ReplicaReplicaGrpcClient {
 private:
  std::unique_ptr<ReplicaReplicaGrpc::Stub> stub_;
  int file_fd;
  string read(const int offset);

 public:
  ReplicaReplicaGrpcClient(std::shared_ptr<Channel> channel,
                           const int& mount_file_fd);
  int clientRestoreData(const vector<int>& offset_v);
  bool clientPrimaryHeartbeat();
};

#endif

/*
How to use this class:
default largest gRPC size = 4MB

const std::string target_str = "128.105.145.95:75247";
ReplicaReplicaGrpcClient * grpcClient = new
ReplicaReplicaGrpcClient(grpc::CreateChannel(target_str,
grpc::InsecureChannelCredentials(), ch_args));

*/