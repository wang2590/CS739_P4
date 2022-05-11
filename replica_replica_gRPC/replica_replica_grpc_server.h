#ifndef REPLICA_REPLICA_GRPC_SERVER_H
#define REPLICA_REPLICA_GRPC_SERVER_H

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>

#include <condition_variable>
#include <mutex>
#include <unordered_map>
#include <vector>

#include "../replica_state.h"
#include "client_replica.grpc.pb.h"
#include "replica_replica.grpc.pb.h"
#include "replica_replica_grpc_client.h"

using grpc::ServerContext;
using grpc::Status;

using common::Empty;
using common::SignedMessage;
using replica_replica::PrePrepareReq;
using replica_replica::ReplicaReplicaGrpc;

class ReplicaReplicaGrpcServiceImpl final : public ReplicaReplicaGrpc::Service {
 public:
  ReplicaReplicaGrpcServiceImpl(ReplicaState* state);
  Status PrePrepare(ServerContext* context, const PrePrepareReq* request,
                    Empty* reply) override;
  Status Prepare(ServerContext* context, const SignedMessage* request,
                 Empty* reply) override;
  Status Commit(ServerContext* context, const SignedMessage* request,
                Empty* reply) override;
  Status RelayRequest(ServerContext* context, const SignedMessage* request,
                      Empty* reply) override;
  Status Checkpoint(ServerContext* context, const SignedMessage* request,
                    Empty* reply) override;

 private:
  ReplicaState* state_;

  struct OperationState {
    client_replica::RequestCmd request;
    std::string digest;
    std::unordered_map<int, SignedMessage> prepare_signatures;
    std::unordered_map<int, SignedMessage> commit_signatures;

    OperationState(client_replica::RequestCmd req, const std::string& d)
        : request(req), digest(d) {}
  };
  std::vector<OperationState> operation_history_;
  std::mutex operation_history_lock_;
  std::condition_variable operation_history_cv_;
};

void RunServer(string serverAddress);

#endif