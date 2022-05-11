#include "client_replica_grpc_server.h"

#include <errno.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <signal.h>

#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "client_replica.grpc.pb.h"

using client_replica::ReplyReq;
using common::Empty;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using namespace std;

ClientReplicaGrpcServiceImpl::ClientReplicaGrpcServiceImpl(ReplicaState* state)
    : state_(state) {}

Status ClientReplicaGrpcServiceImpl::Request(ServerContext* context,
                                             const SignedMessage* request,
                                             Empty* reply) {
  const string msg = request->message();
  const string sig = request->signature();
  // TODO: replica consume client message and signature
  // reply Empty -> nothing
  return Status::OK;
}

Status ClientReplicaGrpcServiceImpl::Reply(
    ServerContext* context, const ReplyReq* request,
    ServerWriter<SignedMessage>* reply_writer) {
  SignedMessage* reply = new SignedMessage();
  const string client_id = request->client_id();
  // TODO: set message and signature
  // reply->set_message();
  // reply->set_signature();

  // TODO: add comsumer
  while (1) {
    reply_writer->Write(*reply);
  }
  return Status::OK;
}
