#include "replica_replica_grpc_client.h"

#include <errno.h>
#include <grpcpp/grpcpp.h>
#include <signal.h>

#include <chrono>
#include <iostream>
#include <memory>
#include <string>

#include "../common.h"
#include "../lib_crypto.h"
#include "replica_replica.grpc.pb.h"

#define TIMEOUT 10 * 1000  // unit in ms, 10 seconds

using grpc::Channel;
using grpc::ClientContext;
// using grpc::ClientReader;
// using grpc::ClientWriter;
using grpc::Status;

using namespace common;
using namespace replica_replica;

ReplicaReplicaGrpcClient::ReplicaReplicaGrpcClient(
    std::shared_ptr<Channel> channel, ReplicaState* state)
    : stub_(ReplicaReplicaGrpc::NewStub(channel)), state_(state) {}

int ReplicaReplicaGrpcClient::ReplicaPrePrepareClient(int32_t v, int64_t n,
                                                      const string& m) {
  PrePrepareCmd cmd;
  cmd.set_v(v);
  cmd.set_n(n);
  cmd.set_d(Sha256Sum(m));

  std::string serilized_cmd = cmd.SerializeAsString();
  if (serilized_cmd == "") return -1;

  PrePrepareReq request;
  request.mutable_preprepare()->set_message(serilized_cmd);
  request.mutable_preprepare()->set_signature(
      SignMessage(serilized_cmd, state_->private_key_path));
  request.set_client_message(m);

  Empty reply;
  ClientContext context;
  Status status = stub_->PrePrepare(&context, request, &reply);

  if (status.ok())
    return 0;
  else
    return status.error_code();
}
int ReplicaReplicaGrpcClient::ReplicaPrepareClient(const string& msg,
                                                   const string& sig) {
  return 0;
}
int ReplicaReplicaGrpcClient::ReplicaCommitClient(const string& msg,
                                                  const string& sig) {
  return 0;
}
int ReplicaReplicaGrpcClient::ReplicaRelayRequestClient(const string& msg,
                                                        const string& sig) {
  return 0;
}
// TODO: checkoint might remove for storing all logs
int ReplicaReplicaGrpcClient::ReplicaCheckpointClient(const string& msg,
                                                      const string& sig) {
  return 0;
}