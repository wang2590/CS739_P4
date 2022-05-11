#ifndef REPLICA_REPLICA_GRPC_CLIENT_H
#define REPLICA_REPLICA_GRPC_CLIENT_H

#include <grpcpp/grpcpp.h>

#include <memory>

#include "../replica_state.h"
#include "replica_replica.grpc.pb.h"

using namespace std;

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
class ReplicaReplicaGrpcClient {
 public:
  ReplicaReplicaGrpcClient(std::shared_ptr<Channel> channel,
                           ReplicaState* state);
  int ReplicaPrePrepareClient(int32_t v, int64_t n, const string& m);
  int ReplicaPrepareClient(int32_t v, int64_t n, const string& d, int32_t i);
  int ReplicaCommitClient(int32_t v, int64_t n, const string& d, int32_t i);
  int ReplicaRelayRequestClient(const common::SignedMessage& request);
  // TODO: checkoint might remove for storing all logs
  int ReplicaCheckpointClient(const string& msg, const string& sig);

 private:
  template <class T>
  int ReplicaSignMessage(const T& proto_cmd, common::SignedMessage* result);

  std::unique_ptr<replica_replica::ReplicaReplicaGrpc::Stub> stub_;
  ReplicaState* state_;
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