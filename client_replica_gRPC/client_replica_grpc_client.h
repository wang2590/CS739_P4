#ifndef CLIENT_REPLICA_GRPC_CLIENT_H
#define CLIENT_SERVER_GRPC_CLIENT_H

#include <grpcpp/grpcpp.h>

#include "client_replica.grpc.pb.h"

using namespace std;
using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using client_server::ClientServergRPC;
using client_server::ReadBlockReply;
using client_server::ReadBlockReq;
using client_server::WriteBlockReply;
using client_server::WriteBlockReq;

class ClientServergRPCClient {
 private:
  std::unique_ptr<ClientServergRPC::Stub> stub_;

 public:
  ClientServergRPCClient(std::shared_ptr<Channel> channel);
  int clientReadBlock(const int& offset, string& buf);
  int clientWriteBlock(const int& offset, const string& buf);
};

#endif

/*
How to use this class:
default largest gRPC size = 4MB

const std::string target_str = "128.105.145.95:75247";
ClientServergRPCClient * grpcClient = new
ClientServergRPCClient(grpc::CreateChannel(target_str,
grpc::InsecureChannelCredentials(), ch_args));

*/