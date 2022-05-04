#include <errno.h>
#include <signal.h>

#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>
// #include <filesystem>
// namespace fs = std::filesystem;

#include "../lib_primary.h"
#include "client_backup_grpc_server.h"

static const string MOUNTPATH = "/p3_block";

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include "client_server.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using client_server::ClientServergRPC;
using client_server::ReadBlockReply;
using client_server::ReadBlockReq;
using client_server::WriteBlockReply;
using client_server::WriteBlockReq;

using namespace std;

extern std::atomic_bool primary_alive;

ClientBackupgRPCServiceImpl::ClientBackupgRPCServiceImpl(
    int mount_file_fd, LibPrimary* lib_primary)
    : mount_file_fd_(mount_file_fd), lib_primary_(lib_primary) {}

Status ClientBackupgRPCServiceImpl::ReadBlock(ServerContext* context,
                                              const ReadBlockReq* request,
                                              ReadBlockReply* reply) {
  std::shared_lock<std::shared_mutex> read_lock(lock_);

  if (primary_alive) {
    reply->set_err(2);
    return Status::OK;
  }

  string buf;
  bool succ = lib_primary_->Read(request->offset(), buf);
  reply->set_buf(buf);
  reply->set_err(!succ);

  // testing code
  if (buf.find("crash_grpc_server_read") != string::npos) {
    // cout << "Killing server process in read\n";
    kill(getpid(), SIGINT);
  }

  return Status::OK;
}
Status ClientBackupgRPCServiceImpl::WriteBlock(ServerContext* context,
                                               const WriteBlockReq* request,
                                               WriteBlockReply* reply) {
  std::unique_lock<std::shared_mutex> write_lock(lock_);

  // testing code
  if (request->buf().find("crash_grpc_server_write") != string::npos) {
    // cout << "Killing server process in write()\n";
    kill(getpid(), SIGINT);
  }

  if (primary_alive) {
    reply->set_err(2);
    return Status::OK;
  }

  bool succ = lib_primary_->Write(request->offset(), request->buf());
  reply->set_err(!succ);

  return Status::OK;
}
