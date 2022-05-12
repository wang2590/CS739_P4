#ifndef REPLICA_REPLICA_GRPC_SERVER_H
#define REPLICA_REPLICA_GRPC_SERVER_H

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>

#include "../replica_state.h"
#include "client_replica.grpc.pb.h"
#include "replica_replica.grpc.pb.h"
#include "replica_replica_grpc_client.h"

using grpc::ServerContext;
using grpc::ServerWriter;
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
  Status Recover(
      ServerContext* context, const replica_replica::RecoverReq* request,
      ServerWriter<replica_replica::RecoverReply>* reply_writer) override;

 private:
  ReplicaState* state_;

  int PerformOperation(const client_replica::OperationCmd& operation_cmd,
                       client_replica::ReplyData* result);
};

void RunServer(string serverAddress);

#endif