#include "primary_backup_grpc_client.h"

#include <errno.h>
#include <grpcpp/grpcpp.h>
#include <signal.h>

#include <chrono>
#include <iostream>
#include <memory>
#include <string>

#include "../common.h"
#include "primary_backup.grpc.pb.h"

#define TIMEOUT 10 * 1000  // unit in ms, 10 seconds

using primary_backup::SyncDataReply;
using primary_backup::SyncDataReq;

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientWriter;
using grpc::Status;

using namespace std;

PrimaryBackupgRPCClient::PrimaryBackupgRPCClient(
    std::shared_ptr<Channel> channel, const int& mount_file_fd)
    : stub_(PrimaryBackupgRPC::NewStub(channel)), file_d(mount_file_fd) {}

int PrimaryBackupgRPCClient::clientSyncData(const vector<int>& offset_v) {
  cout << "gRPC client SyncData write" << endl;
  SyncDataReq request;
  SyncDataReply reply;
  ClientContext context;
  std::chrono::time_point deadline =
      std::chrono::system_clock::now() + std::chrono::milliseconds(TIMEOUT);
  context.set_deadline(deadline);
  unique_ptr<ClientWriter<SyncDataReq>> writer(
      stub_->SyncData(&context, &reply));
  for (int i : offset_v) {
    std::string buf = read(i);
    request.set_buf(buf);
    request.set_offset(i);
    if (!writer->Write(request)) {
      // Broken stream.
      break;
    }
    // crash testing code
    if (buf.find("crash_syncData") != string::npos) {
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

string PrimaryBackupgRPCClient::read(const int offset) {
  string buf;
  buf.resize(kBlockSize);
  if (pread(file_d, buf.data(), kBlockSize, offset) < 0) {
    perror("pread");
  }
  return buf;
}
