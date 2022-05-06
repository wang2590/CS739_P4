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

using namespace std;

ReplicaReplicaGrpcServiceImpl::ReplicaReplicaGrpcServiceImpl(
    ReplicaState* state)
    : state_(state) {}

Status ReplicaReplicaGrpcServiceImpl::PrePrepare(ServerContext* context,
                                                 const PrePrepareReq* request,
                                                 Empty* reply) {
  return Status::OK;
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