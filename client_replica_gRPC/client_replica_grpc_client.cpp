#include "client_replica_grpc_client.h"

#include <errno.h>
#include <grpcpp/grpcpp.h>
#include <signal.h>

#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

#include "../lib_crypto.h"
#include "client_replica.grpc.pb.h"

#define TIMEOUT 10 * 1000  // unit in ms, 10 seconds

using namespace client_replica;
using namespace common;
using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::Status;

using namespace std;

ClientReplicaGrpcClient::ClientReplicaGrpcClient(
    std::shared_ptr<Channel> channel, ClientState* state, int id)
    : stub_(ClientReplicaGrpc::NewStub(channel)),
      state_(state),
      replicaID(id) {}

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

void ClientReplicaGrpcClient::clientReply() {
  ReplyReq request;
  request.set_client_id(state_->public_key);
  SignedMessage reply;
  ClientContext context;
  unique_ptr<ClientReader<SignedMessage>> reader(
      stub_->Reply(&context, request));
  while (reader->Read(&reply)) {
    ReplyCmd result;
    if (VerifyAndDecodeMessage(
            reply, state_->replicas_public_keys[replicaID].get(), &result) &&
        replicaID == result.i()) {
      state_->q->do_fill(result);
    } else {
      cout << "Error in reply Msg from Replica: " << replicaID << endl;
    }
  }
  // Technitically shuold never finish
  Status status = reader->Finish();
}

std::thread ClientReplicaGrpcClient::thread_func() {
  return std::thread([=] { clientReply(); });
}