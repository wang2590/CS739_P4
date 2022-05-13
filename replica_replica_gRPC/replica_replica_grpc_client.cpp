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
using grpc::ClientReader;
using grpc::Status;

using namespace common;
using namespace replica_replica;

ReplicaReplicaGrpcClient::ReplicaReplicaGrpcClient(
    std::shared_ptr<Channel> channel, ReplicaState* state)
    : stub_(ReplicaReplicaGrpc::NewStub(channel)), state_(state) {}

int ReplicaReplicaGrpcClient::ReplicaPrePrepareClient(int32_t v, int64_t n,
                                                      const SignedMessage& m,
                                                      std::string d) {
  PrePrepareCmd cmd;
  cmd.set_v(v);
  cmd.set_n(n);
  cmd.set_d(d);

  PrePrepareReq request;
  if (ReplicaSignMessage(cmd, request.mutable_preprepare()) < 0) return -1;
  request.mutable_client_message()->CopyFrom(m);

  std::thread t([=]() {
    Empty reply;
    ClientContext context;
    Status status = stub_->PrePrepare(&context, request, &reply);
  });
  t.detach();

  return 0;
}

int ReplicaReplicaGrpcClient::ReplicaPrepareClient(int32_t v, int64_t n,
                                                   const string& d, int32_t i) {
  PrepareCmd cmd;
  cmd.set_v(v);
  cmd.set_n(n);
  cmd.set_d(d);
  cmd.set_i(i);

  SignedMessage request;
  if (ReplicaSignMessage(cmd, &request) < 0) return -1;

  std::thread t([=]() {
    Empty reply;
    ClientContext context;
    Status status = stub_->Prepare(&context, request, &reply);
  });
  t.detach();

  return 0;
}

int ReplicaReplicaGrpcClient::ReplicaCommitClient(int32_t v, int64_t n,
                                                  const string& d, int32_t i) {
  CommitCmd cmd;
  cmd.set_v(v);
  cmd.set_n(n);
  cmd.set_d(d);
  cmd.set_i(i);

  SignedMessage request;
  if (ReplicaSignMessage(cmd, &request) < 0) return -1;

  std::thread t([=]() {
    Empty reply;
    ClientContext context;
    Status status = stub_->Commit(&context, request, &reply);
  });
  t.detach();

  return 0;
}

int ReplicaReplicaGrpcClient::ReplicaRelayRequestClient(
    const SignedMessage& request) {
  std::thread t([=]() {
    Empty reply;
    ClientContext context;
    Status status = stub_->RelayRequest(&context, request, &reply);
  });
  t.detach();

  return 0;
}

template <class T>
int ReplicaReplicaGrpcClient::ReplicaSignMessage(const T& proto_cmd,
                                                 SignedMessage* result) {
  return SignMessage(proto_cmd, state_->private_key.get(), result);
}
