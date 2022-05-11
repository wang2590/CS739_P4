#ifndef CLIENT_REPLICA_GRPC_CLIENT_H
#define CLIENT_REPLICA_GRPC_CLIENT_H

#include <grpcpp/grpcpp.h>

#include "../client_state.h"
#include "../consumer_queue.h"
#include "client_replica.grpc.pb.h"

using namespace std;
using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using client_replica::ClientReplicaGrpc;
using client_replica::RequestCmd;
using common::Empty;
using common::SignedMessage;

class ClientReplicaGrpcClient {
 private:
  std::unique_ptr<ClientReplicaGrpc::Stub> stub_;
  ClientState* state_;

 public:
  ClientReplicaGrpcClient(std::shared_ptr<Channel> channel, ClientState* state);
  int clientRequest(const RequestCmd& cmd);
  int clientReply(const string& clientPubKey);
};

#endif
