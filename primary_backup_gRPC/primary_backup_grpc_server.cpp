#include "primary_backup_grpc_server.h"

#include <errno.h>
#include <signal.h>

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

#include "primary_backup.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using primary_backup::PrimaryBackupgRPC;
using primary_backup::SyncDataReply;
using primary_backup::SyncDataReq;
using namespace std;

Status PrimaryBackupgRPCServiceImpl::SyncData(ServerContext *context,
                                              ServerReader<SyncDataReq> *reader,
                                              SyncDataReply *reply) {
  SyncDataReq request;
  bool err = false;
  while (reader->Read(&request)) {
    // testing code
    if (request.buf().find("crash_primary_backup_grpc_server") !=
        string::npos) {
      // cout << "Killing server process in write()\n";
      kill(getpid(), SIGINT);
    }

    assert(request.buf().size() == kBlockSize);

    if (pwrite(mount_file_fd_, request.buf().data(), kBlockSize,
               request.offset()) < 0) {
      perror("pwrite");
      err = true;
    } else if (fsync(mount_file_fd_) < 0) {
      perror("perror");
      err = true;
    }
  }
  reply->set_err(err);

  return Status::OK;
}