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
using client_replica::ReplyReq;
using common::Empty;
using common::SignedMessage;
using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using namespace std;

ClientReplicaGrpcClient::ClientReplicaGrpcClient(
    std::shared_ptr<Channel> channel)
    : stub_(ClientReplicaGrpc::NewStub(channel)) {}

int ClientReplicaGrpcClient::clientRequest(const string& msg,
                                           const string& sig) {
  SignedMessage request;
  request.set_message(msg);
  request.set_signature(sig);

  Empty reply;
  ClientContext context;
  Status status = stub_->Request(&context, request, &reply);

  // // testing code
  // if (reply.buf().find("crash_client_server_grpc_client_read") !=
  //     string::npos) {
  //   cout << "Killing client process in read()\n";
  //   kill(getpid(), SIGABRT);
  // }

  if (status.ok()) {
    return 0;
  }
  cout << "There was an error in client request: " << status.error_message()
       << endl;
  return status.error_code();
}
int ClientReplicaGrpcClient::clientReply(const string& clientPubKey) {
  ReplyReq request;
  request.set_client_id(clientPubKey);
  SignedMessage reply;
  ClientContext context;
  Status status = stub_->Reply(&context, request, &reply);

  // // testing code
  // if (reply.buf().find("crash_client_server_grpc_client_read") !=
  //     string::npos) {
  //   cout << "Killing client process in read()\n";
  //   kill(getpid(), SIGABRT);
  // }
  buf.append(reply.buf());

  if (status.ok()) {
    return 0;
  }
  cout << "There was an error in client request: " << status.error_message()
       << endl;
  return status.error_code();
}
