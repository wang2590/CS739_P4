#ifndef REPLICA_REPLICA_GRPC_SERVER_H
#define REPLICA_REPLICA_GRPC_SERVER_H

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>

#include "replica_replica.grpc.pb.h"
#include "replica_replica_grpc_client.h"

using namespace std;

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
// using grpc::ServerReader;
using grpc::Status;

using replica_replica::ReplicaReplicaGrpc;

class ReplicaReplicaGrpcServiceImpl final : public ReplicaReplicaGrpc::Service {
 public:
  ReplicaReplicaGrpcServiceImpl(int mount_file_fd);
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
  int mount_file_fd_;
  // ReplicaReplicaGrpcClient* primary_backup_client_;
};

void RunServer(string serverAddress);

#endif