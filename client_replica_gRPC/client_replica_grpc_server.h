#ifndef CLIENT_REPLICA_GRPC_SERVER_H
#define CLIENT_REPLICA_GRPC_SERVER_H

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include <shared_mutex>

#include "client_replica.grpc.pb.h"

using namespace std;

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerWriter;
using grpc::Status;

using client_replica::ClientReplicaGrpc;
using client_replica::Empty;
using client_replica::SignedMessage;

class ClientReplicaGrpcServiceImpl final : public ClientReplicaGrpc::Service {
 public:
  ClientReplicaGrpcServiceImpl(int mount_file_fd);
  Status Request(ServerContext* context, const SignedMessage* request,
                 Empty* reply) override;
  Status Reply(ServerContext* context, const Empty* request,
               ServerWriter<SignedMessage>* reply_writer) override;

 private:
  int mount_file_fd_;
  std::shared_mutex lock_;
};

void RunServer(string serverAddress);

#endif