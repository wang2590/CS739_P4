#ifndef CLIENT_REPLICA_GRPC_SERVER_H
#define CLIENT_REPLICA_GRPC_SERVER_H

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include "../lib_crypto.h"
#include "../replica_replica_gRPC/replica_replica_grpc_client.h"
#include "../replica_state.h"
#include "client_replica.grpc.pb.h"

using namespace std;

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerWriter;
using grpc::Status;

using client_replica::ClientReplicaGrpc;
using client_replica::ReplyReq;
using common::Empty;
using common::SignedMessage;
class ClientReplicaGrpcServiceImpl final : public ClientReplicaGrpc::Service {
 public:
  ClientReplicaGrpcServiceImpl(ReplicaState* state);
  Status Request(ServerContext* context, const SignedMessage* request,
                 Empty* reply) override;
  Status Reply(ServerContext* context, const ReplyReq* request,
               ServerWriter<SignedMessage>* reply_writer) override;

 private:
  SignedMessage CreateFakeRequest(client_replica::RequestCmd request_cmd);

  ReplicaState* state_;
};

void RunServer(string serverAddress);

#endif