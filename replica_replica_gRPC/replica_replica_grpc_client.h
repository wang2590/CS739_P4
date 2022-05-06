#ifndef REPLICA_REPLICA_GRPC_CLIENT_H
#define REPLICA_REPLICA_GRPC_CLIENT_H

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

 public:
  ReplicaReplicaGrpcClient(std::shared_ptr<Channel> channel);
  int ReplicaPrePrepareClient(const string& msg, const string& sig,
                              const string& client_msg);
  int ReplicaPrepareClient(const string& msg, const string& sig);
  int ReplicaCommitClient(const string& msg, const string& sig);
  int ReplicaRelayRequestClient(const string& msg, const string& sig);
  // TODO: checkoint might remove for storing all logs
  int ReplicaCheckpointClient(const string& msg, const string& sig);
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