#include "replica_replica_grpc_server.h"

#include <errno.h>
#include <fcntl.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <signal.h>
#include <unistd.h>

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "../common.h"
#include "replica_replica.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::StatusCode;

using namespace client_replica;
using namespace replica_replica;

ReplicaReplicaGrpcServiceImpl::ReplicaReplicaGrpcServiceImpl(
    ReplicaState* state)
    : state_(state) {}

Status ReplicaReplicaGrpcServiceImpl::PrePrepare(ServerContext* context,
                                                 const PrePrepareReq* request,
                                                 Empty* reply) {
  if (state_->replica_id == state_->primary) {
    // The primary itself should not receive such message. Ignore it.
    return Status(StatusCode::PERMISSION_DENIED,
                  "Only the primary can send pre-prepare.");
  } else {
    const SignedMessage& preprepare = request->preprepare();
    const SignedMessage& client_message = request->client_message();

    // ==== Verify the authenticity of pre-prepare message ====

    PrePrepareCmd preprepare_cmd;
    if (!VerifyAndDecodeMessage(
            preprepare, state_->replicas_public_keys[state_->primary].get(),
            &preprepare_cmd)) {
      // This pre-prepare is not signed properly by the primary
      return Status(StatusCode::PERMISSION_DENIED,
                    "Only the primary can send pre-prepare.");
    }

    RequestCmd client_cmd;
    client_cmd.ParseFromString(client_message.message());
    RsaPtr rsa = CreateRsa(client_cmd.c(), true);
    if (!VerifyMessage(client_message.message(), client_message.signature(),
                       rsa.get())) {
      goto faulty_primary;
    }

    if (preprepare_cmd.d() == Sha256Sum(client_message.message())) {
      goto faulty_primary;
    }

    if (preprepare_cmd.v() != state_->view) {
      goto faulty_primary;
    }

    // ==== Check n and add the request to history ====
    {
      const std::lock_guard<std::mutex> lock(operation_history_lock_);
      if (preprepare_cmd.n() != operation_history_.size()) {
        goto faulty_primary;
      }
      operation_history_.push_back(client_cmd);
    }

    // ==== Send prepare message ====

    for (const auto& replica_client : state_->replica_clients) {
      replica_client->ReplicaPrepareClient(
          preprepare_cmd.v(), preprepare_cmd.n(), preprepare_cmd.d(),
          state_->replica_id);
    }
  faulty_primary:
    // TODO: view change
    return Status(StatusCode::PERMISSION_DENIED,
                  "Only the primary can send pre-prepare.");
    return Status::OK;
  }
}

Status ReplicaReplicaGrpcServiceImpl::Prepare(ServerContext* context,
                                              const SignedMessage* request,
                                              Empty* reply) {
  return Status::OK;
}
Status ReplicaReplicaGrpcServiceImpl::Commit(ServerContext* context,
                                             const SignedMessage* request,
                                             Empty* reply) {
  return Status::OK;
}
Status ReplicaReplicaGrpcServiceImpl::RelayRequest(ServerContext* context,
                                                   const SignedMessage* request,
                                                   Empty* reply) {
  return Status::OK;
}
Status ReplicaReplicaGrpcServiceImpl::Checkpoint(ServerContext* context,
                                                 const SignedMessage* request,
                                                 Empty* reply) {
  return Status::OK;
}

// // Run this in Primary/backup's main function
// void RunServer(string serverAddress) {
//   ReplicaReplicaGrpcServiceImpl service1;
//   // ClientServergRPCServiceImpl service2;
//   grpc::EnableDefaultHealthCheckService(true);
//   grpc::reflection::InitProtoReflectionServerBuilderPlugin();
//   ServerBuilder builder;
//   builder.AddListeningPort(serverAddress, grpc::InsecureServerCredentials());
//   // multiple services can be registered
//   //
//   https://github.com/grpc/grpc/blob/master/test/cpp/end2end/hybrid_end2end_test.cc
//   builder.RegisterService(&service1);
//   // builder.RegisterService(&service2);
//   std::unique_ptr<Server> server(builder.BuildAndStart());
//   std::cout << "Server listening on " << serverAddress << std::endl;
//   server->Wait();
// }

// int main(int argc, char** argv) {
//   string serverAddress("0.0.0.0:50051");
//   RunServer(serverAddress);
//   return 0;
// }