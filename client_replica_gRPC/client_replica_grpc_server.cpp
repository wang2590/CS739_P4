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
using namespace client_replica;
using grpc::StatusCode;

ClientReplicaGrpcServiceImpl::ClientReplicaGrpcServiceImpl(ReplicaState* state)
    : state_(state) {}

Status ClientReplicaGrpcServiceImpl::Request(ServerContext* context,
                                             const SignedMessage* request,
                                             Empty* reply) {
  RequestCmd client_cmd;
  if (client_cmd.ParseFromString(request->message())) {
    RsaPtr rsa = CreateRsa(client_cmd.c(), true);
    if (VerifyMessage(request->message(), request->signature(), rsa.get())) {
      std::string digest = Sha256Sum(request->message());
      int sequence_n = 0;
      {
        const std::lock_guard<std::mutex> lock(state_->operation_history_lock);
        sequence_n = state_->operation_history.size();
        state_->operation_history.emplace_back(client_cmd, digest);
      }
      for (auto& i : state_->replica_clients) {
        i->ReplicaPrePrepareClient(state_->view, sequence_n, *request, digest);
      }
    } else {
      return Status(StatusCode::PERMISSION_DENIED, "Wrong request type!");
    }
    // reply Empty -> nothing
    return Status::OK;
  }
}
Status ClientReplicaGrpcServiceImpl::Reply(
    ServerContext* context, const ReplyReq* request,
    ServerWriter<SignedMessage>* reply_writer) {
  // unsure about the client id where it goes
  const string client_id = request->client_id();
  // ReplyCmd Comsumer
  while (1) {                                                  // infinite loop
    auto time_out = std::chrono::system_clock::now() + 9999s;  // scary ):
    ReplyCmd result;
    state_->replies.do_get(time_out, result);
    SignedMessage* reply = new SignedMessage();
    if (SignMessage(result, state_->private_key.get(), reply) < 0)
      return Status(StatusCode::PERMISSION_DENIED, "Wrong request type!");
    reply_writer->Write(*reply);
  }
  return Status::OK;
}
