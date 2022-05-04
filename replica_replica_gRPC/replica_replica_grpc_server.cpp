#include "backup_primary_grpc_server.h"

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "../common.h"
#include "../lib_primary.h"

static const string MOUNTPATH = "/p3_block";

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include "backup_primary.grpc.pb.h"

#define CHECK_ERR(S, MSG) \
  if ((S) < 0) {          \
    perror(MSG);          \
    reply->set_err(1);    \
    return Status::OK;    \
  }

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using backup_primary::BackupPrimarygRPC;
using backup_primary::PrimaryHeatbeatReply;
using backup_primary::PrimaryHeatbeatReq;
using backup_primary::RestoreDataReply;
using backup_primary::RestoreDataReq;
using namespace std;

BackupPrimarygRPCServiceImpl::BackupPrimarygRPCServiceImpl(
    int mount_file_fd, LibPrimary* lib_primary,
    PrimaryBackupgRPCClient* primary_backup_client)
    : mount_file_fd_(mount_file_fd),
      lib_primary_(lib_primary),
      primary_backup_client_(primary_backup_client) {}

Status BackupPrimarygRPCServiceImpl::RestoreData(
    ServerContext* context, ServerReader<RestoreDataReq>* reader,
    RestoreDataReply* reply) {
  int tmp_offset = -1;
  int tmp_fd = -1;
  std::string tmp_buf;
  if (access(kTempFilesPath, F_OK) == 0) {  // Temp file exists
    tmp_fd = open(kTempFilesPath, O_RDONLY);
    CHECK_ERR(tmp_fd, "open");
    CHECK_ERR(
        read(tmp_fd, reinterpret_cast<char*>(&tmp_offset), sizeof(tmp_offset)),
        "read");
    tmp_buf.resize(kBlockSize);
    int sz = read(tmp_fd, tmp_buf.data(), kBlockSize);
    CHECK_ERR(sz, "read");
    if (sz < kBlockSize) {
      tmp_fd = -1;
    }
    CHECK_ERR(close(tmp_fd), "close");
    CHECK_ERR(unlink(kTempFilesPath), "unlink");
  }

  RestoreDataReq request;
  while (reader->Read(&request)) {
    // testing code
    if (request.buf().find("crash_backup_primary_grpc_serverc") !=
        string::npos) {
      // cout << "Killing server process in write()\n";
      kill(getpid(), SIGINT);
    }

    if (request.buf().size() != kBlockSize) {
      reply->set_err(2);
      return Status::OK;
    }
    if (pwrite(mount_file_fd_, request.buf().data(), kBlockSize,
               request.offset()) < 0) {
      perror("pwrite");
      reply->set_err(1);
    }

    if (tmp_fd != -1 && request.offset() == tmp_offset) {
      tmp_fd = -1;
    }
  }

  if (tmp_fd != -1) {
    CHECK_ERR(pwrite(mount_file_fd_, tmp_buf.data(), kBlockSize, tmp_offset),
              "pwrite");
    primary_backup_client_->clientSyncData({tmp_offset});
  }

  return Status::OK;
}

Status BackupPrimarygRPCServiceImpl::PrimaryHeartbeat(
    ServerContext* context, const PrimaryHeatbeatReq* request,
    PrimaryHeatbeatReply* reply) {
  lib_primary_->BackupAlive();

  reply->set_beat(request->beat());
  return Status::OK;
}

// // Run this in Primary/backup's main function
// void RunServer(string serverAddress) {
//   BackupPrimarygRPCServiceImpl service1;
//   // ClientServergRPCServiceImpl service2;
//   grpc::EnableDefaultHealthCheckService(true);
//   grpc::reflection::InitProtoReflectionServerBuilderPlugin();
//   ServerBuilder builder;
//   builder.AddListeningPort(serverAddress, grpc::InsecureServerCredentials());
//   // multiple services can be registered
//   //
//   https://github.com/grpc/grpc/blob/master/test/cpp/end2end/hybrid_end2end_test.cc
//   builder.RegisterService(&service1);
//   // builder.RegisterService(&service2);
//   std::unique_ptr<Server> server(builder.BuildAndStart());
//   std::cout << "Server listening on " << serverAddress << std::endl;
//   server->Wait();
// }

// int main(int argc, char** argv) {
//   string serverAddress("0.0.0.0:50051");
//   RunServer(serverAddress);
//   return 0;
// }