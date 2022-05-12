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
                                                      const string& m) {
  PrePrepareCmd cmd;
  cmd.set_v(v);
  cmd.set_n(n);
  cmd.set_d(Sha256Sum(m));

  PrePrepareReq request;
  if (ReplicaSignMessage(cmd, request.mutable_preprepare()) < 0) return -1;
  request.mutable_client_message()->set_message(m);
  request.mutable_client_message()->set_signature(
      SignMessage(m, state_->private_key.get()));

  Empty reply;
  ClientContext context;
  Status status = stub_->PrePrepare(&context, request, &reply);

  if (status.ok())
    return 0;
  else
    return status.error_code();
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

  Empty reply;
  ClientContext context;
  Status status = stub_->Prepare(&context, request, &reply);

  if (status.ok())
    return 0;
  else
    return status.error_code();
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

  Empty reply;
  ClientContext context;
  Status status = stub_->Commit(&context, request, &reply);

  if (status.ok())
    return 0;
  else
    return status.error_code();
}

int ReplicaReplicaGrpcClient::ReplicaRelayRequestClient(
    const SignedMessage& request) {
  Empty reply;
  ClientContext context;
  Status status = stub_->RelayRequest(&context, request, &reply);

  if (status.ok())
    return 0;
  else
    return status.error_code();
}

template <class T>
int ReplicaReplicaGrpcClient::ReplicaSignMessage(const T& proto_cmd,
                                                 SignedMessage* result) {
  return SignMessage(proto_cmd, state_->private_key.get(), result);
}

int ReplicaReplicaGrpcClient::ReplicaRecoverClient(int last_n) {
  RecoverReq request;
  request.set_last_n(last_n);
  RecoverReply reply;
  ClientContext context;
  std::unique_ptr<ClientReader<RecoverReply>> reader(
      stub_->Recover(&context, request));

  while (reader->Read(&reply)) {
    std::set<int> signed_replicas;
    for (const SignedMessage& signed_message : request) {
      CommitCmd commit_cmd;
      commit_cmd.ParseFromString(signed_message.message());

      std::string res =
          state_->rr_service->VerifyAndStoreCommit(&signed_message);
      if (res == "") {
        signed_replicas.insert(commit_cmd.i());
      }
    }

    if (signed_replicas.size() >= 2 * state_.f + 1) {
      std::lock_guard<std::mutex> lock(state_->operation_history_lock);
      OperationState& op = state_->operation_history[commit_cmd.n()];

      std::unique_lock<std::mutex> last_commit_lock(
          state_->last_commited_operation_lock);

      while (state_->last_commited_operation != commit_cmd.n() - 1) {
        state_->last_commited_operation_cv.wait(last_commit_lock);
      }

      ReplyCmd reply_cmd;
      reply_cmd.set_v(state_->view);
      reply_cmd.set_t(op.request.t());
      reply_cmd.set_c(op.request.c());
      reply_cmd.set_i(state_->replica_id);

      if (PerformOperation(op.request.o(), reply_cmd.mutable_r()) != 0) {
        return Status(StatusCode::INTERNAL, "Failed to perform the operation");
      }

      state_->replies.do_fill(reply_cmd);

      state_->last_commited_operation = commit_cmd.n();
      state_->last_commited_operation_cv.notify_all();
    }
  }

  Status status = reader->Finish();
}
