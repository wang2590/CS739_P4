#include "client_replica_grpc_client.h"

#include <errno.h>
#include <grpcpp/grpcpp.h>
#include <signal.h>

#include <chrono>
#include <iostream>
#include <memory>
#include <string>

#include "client_replica.grpc.pb.h"

#define TIMEOUT 10 * 1000  // unit in ms, 10 seconds

using client_replica::ClientReplicaGrpc;
using client_replica::Empty;
using client_replica::SignedMessage;

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using namespace std;

ClientReplicaGrpcClient::ClientReplicaGrpcClient(
    std::shared_ptr<Channel> channel)
    : stub_(ClientReplicaGrpc::NewStub(channel)) {}

int clientRequest(const string& msg, const string& sig) { return 0; }
int clientReply() { return 0; }
