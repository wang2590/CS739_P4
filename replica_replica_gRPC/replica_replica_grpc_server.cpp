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
    std::cout
        << "The primary should not receive a pre-prepare message. Ignore it."
        << std::endl;
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
      std::cout << "This pre-prepare is not signed properly by the primary."
                << std::endl;
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
      // TODO: Check if n is between low and high water mark
      const std::lock_guard<std::mutex> lock(operation_history_lock_);
      if (preprepare_cmd.n() != operation_history_.size()) {
        goto faulty_primary;
      }
      operation_history_.emplace_back(client_cmd, preprepare_cmd.d());
    }
    operation_history_cv_.notify_all();

    // ==== Send prepare message ====

    for (const auto& replica_client : state_->replica_clients) {
      if (replica_client == nullptr) continue;
      if (replica_client->ReplicaPrepareClient(
              preprepare_cmd.v(), preprepare_cmd.n(), preprepare_cmd.d(),
              state_->replica_id) != 0) {
        // We can ignore the error because it does not need to take action for
        // a faulty replica.
        std::cout << "Fail to send prepare message to a replica. Ignore it."
                  << std::endl;
      }
    }
    return Status::OK;

  faulty_primary:
    // TODO: view change
    std::cout << "Faulty primary." << std::endl;
    return Status(StatusCode::PERMISSION_DENIED,
                  "Only the primary can send pre-prepare.");
  }
}

Status ReplicaReplicaGrpcServiceImpl::Prepare(ServerContext* context,
                                              const SignedMessage* request,
                                              Empty* reply) {
  // ==== Verify the authenticity of prepare message ====

  PrepareCmd preprepare_cmd;
  preprepare_cmd.ParseFromString(request->message());

  if (preprepare_cmd.i() >= state_->replicas_public_keys.size()) {
    std::cout << "Invalid replica number in the prepare message." << std::endl;
    return Status(StatusCode::PERMISSION_DENIED, "Invalid replica number");
  }

  if (!VerifyMessage(request->message(), request->signature(),
                     state_->replicas_public_keys[preprepare_cmd.i()].get())) {
    std::cout << "Incorrect signature of the prepare message." << std::endl;
    return Status(StatusCode::PERMISSION_DENIED, "Incorrect signature");
  }

  if (preprepare_cmd.v() != state_->view) {
    std::cout << "Incorrect view number in the prepare message." << std::endl;
    return Status(StatusCode::PERMISSION_DENIED, "Incorrect view number");
  }

  std::unique_lock<std::mutex> lock(operation_history_lock_);
  while (preprepare_cmd.n() < operation_history_.size()) {
    operation_history_cv_.wait(lock);
  }

  OperationState& op = operation_history_[preprepare_cmd.n()];
  if (preprepare_cmd.d() != op.digest) {
    std::cout << "Incorrect digest in the prepare message." << std::endl;
    return Status(StatusCode::PERMISSION_DENIED, "Incorrect digest");
  }

  // ==== Store the signatures ====

  op.prepare_signatures[preprepare_cmd.i()] = *request;

  // ==== Send commit message ====

  if (op.prepare_signatures.size() >= 2 * state_->f) {
    for (const auto& replica_client : state_->replica_clients) {
      if (replica_client == nullptr) continue;
      if (replica_client->ReplicaCommitClient(
              preprepare_cmd.v(), preprepare_cmd.n(), preprepare_cmd.d(),
              state_->replica_id) != 0) {
        // We can ignore the error because it does not need to take action for
        // a faulty replica.
        std::cout << "Fail to send commit message to a replica. Ignore it."
                  << std::endl;
      }
    }
  }

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