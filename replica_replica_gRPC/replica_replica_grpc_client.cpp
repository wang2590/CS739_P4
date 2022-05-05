#include "replica_replica_grpc_client.h"

#include <errno.h>
#include <grpcpp/grpcpp.h>
#include <signal.h>

#include <chrono>
#include <iostream>
#include <memory>
#include <string>

#include "../common.h"
#include "replica_replica.grpc.pb.h"

#define TIMEOUT 10 * 1000  // unit in ms, 10 seconds
#define HEARTBEAT 6        // random integer

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientWriter;
using grpc::Status;

using replica_replica::ReplicaReplicaGrpc;
using replica_replica::ReplicaReplicaGrpcClient;

using namespace std;

ReplicaReplicaGrpcClient::ReplicaReplicaGrpcClient(
    std::shared_ptr<Channel> channel, const int& mount_file_fd)
    : stub_(ReplicaReplicaGrpc::NewStub(channel)), file_fd(mount_file_fd) {}

int ReplicaReplicaGrpcClient::clientRestoreData(const vector<int>& offset_v) {
  cout << "backup primary grpc client restore data write" << endl;
  RestoreDataReq request;
  RestoreDataReply reply;
  ClientContext context;
  std::chrono::time_point deadline =
      std::chrono::system_clock::now() + std::chrono::milliseconds(TIMEOUT);
  context.set_deadline(deadline);
  unique_ptr<ClientWriter<RestoreDataReq>> writer(
      stub_->RestoreData(&context, &reply));
  for (int i : offset_v) {
    std::string buf = read(i);
    request.set_buf(buf);
    request.set_offset(i);
    if (!writer->Write(request)) {
      // Broken stream.
      break;
    }
    // crash testing code
    if (buf.find("crash_replica_replica_grpc_client_restore_data") !=
        string::npos) {
      cout << "Killing client process in write()\n";
      kill(getpid(), SIGABRT);
    }
  }

  writer->WritesDone();
  Status status = writer->Finish();

  if (status.ok()) {
    return reply.err();
  }
  cout << "There was an error in the server Write " << status.error_code()
       << endl;
  return status.error_code();
}

bool ReplicaReplicaGrpcClient::clientPrimaryHeartbeat() {
  cout << "primary heartbeat check client " << endl;
  PrimaryHeatbeatReq request;
  request.set_beat(HEARTBEAT);

  PrimaryHeatbeatReply reply;
  ClientContext context;
  std::chrono::time_point deadline =
      std::chrono::system_clock::now() +
      std::chrono::milliseconds(TIMEOUT / 2);  // faster tiemput than write
  context.set_deadline(deadline);
  Status status = stub_->PrimaryHeartbeat(&context, request, &reply);

  // // testing code
  // if (test.find("crash_primary_heartbeat_check_client") != string::npos)
  // {
  //     cout << "Killing client process in read()\n";
  //     kill(getpid(), SIGABRT);
  // }

  if (status.ok() && reply.beat() == HEARTBEAT) {
    return true;
  }
  cout << "Primary heartbeat stoppted! " << status.error_code() << endl;
  return false;
}

string ReplicaReplicaGrpcClient::read(const int offset) {
  string buf;
  buf.resize(kBlockSize);
  if (pread(file_fd, buf.data(), kBlockSize, offset) < 0) {
    perror("pread");
  }
  return buf;
}