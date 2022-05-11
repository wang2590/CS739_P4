#include "client_replica_grpc_client.h"

#include <errno.h>
#include <grpcpp/grpcpp.h>
#include <signal.h>

#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

#include "client_replica.grpc.pb.h"
#include "../lib_crypto.h"

#define TIMEOUT 10 * 1000  // unit in ms, 10 seconds

using namespace client_replica;
using namespace common;
using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::Status;

using namespace std;

ClientReplicaGrpcClient::ClientReplicaGrpcClient(
    std::shared_ptr<Channel> channel, ClientState* state)
    : stub_(ClientReplicaGrpc::NewStub(channel)), state_(state) {}

int ClientReplicaGrpcClient::clientRequest(const RequestCmd& cmd) {
  SignedMessage request;
  if (SignMessage(cmd, state_->private_key.get(), &request) < 0) return -1;

  Empty reply;
  ClientContext context;
  Status status = stub_->Request(&context, request, &reply);

  if (status.ok())
    return 0;
  else
    return status.error_code();
}

int ClientReplicaGrpcClient::clientReply(const string& clientPubKey) {
  ReplyReq request;
  request.set_client_id(clientPubKey);
  SignedMessage reply;
  ClientContext context;
  unique_ptr<ClientReader<SignedMessage>> reader(
      stub_->Reply(&context, request));
  while (reader->Read(&reply)) {
    state_->q->do_fill(make_pair(reply.message(), reply.signature()));
  }
  Status status = reader->Finish();
  if (status.ok()) {
    return 0;
  }
  cout << "There was an error in client request: " << status.error_message()
       << endl;
  return status.error_code();
}
