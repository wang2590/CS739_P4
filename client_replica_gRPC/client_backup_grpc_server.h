#ifndef CLIENT_SERVER_GRPC_SERVER_H
#define CLIENT_SERVER_GRPC_SERVER_H

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include <shared_mutex>

#include "../lib_primary.h"
#include "client_server.grpc.pb.h"

using namespace std;

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using client_server::ClientServergRPC;
using client_server::ReadBlockReply;
using client_server::ReadBlockReq;
using client_server::WriteBlockReply;
using client_server::WriteBlockReq;

class ClientBackupgRPCServiceImpl final : public ClientServergRPC::Service {
 public:
  ClientBackupgRPCServiceImpl(int mount_file_fd, LibPrimary* lib_primary);
  Status ReadBlock(ServerContext* context, const ReadBlockReq* request,
                   ReadBlockReply* reply) override;
  Status WriteBlock(ServerContext* context, const WriteBlockReq* request,
                    WriteBlockReply* reply) override;

 private:
  int mount_file_fd_;
  LibPrimary* lib_primary_;
  std::shared_mutex lock_;
};

#endif