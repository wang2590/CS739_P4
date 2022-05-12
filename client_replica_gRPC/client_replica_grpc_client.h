#ifndef CLIENT_REPLICA_GRPC_CLIENT_H
#define CLIENT_REPLICA_GRPC_CLIENT_H

#include <grpcpp/grpcpp.h>

#include <thread>

#include "../client_state.h"
#include "../consumer_queue.h"
#include "../lib_crypto.h"
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
  int replicaID;

 public:
  ClientReplicaGrpcClient(std::shared_ptr<Channel> channel, ClientState* state,
                          int id);
  int clientRequest(const client_replica::RequestCmd& cmd);
  void clientReply();
  std::thread thread_func();
};

#endif
