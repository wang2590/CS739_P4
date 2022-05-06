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

using grpc::Channel;
using grpc::ClientContext;
// using grpc::ClientReader;
// using grpc::ClientWriter;
using grpc::Status;

using replica_replica::Empty;
using replica_replica::PrePrepareReq;
using replica_replica::ReplicaReplicaGrpc;
using replica_replica::SignedMessage;
using namespace std;

ReplicaReplicaGrpcClient::ReplicaReplicaGrpcClient(
    std::shared_ptr<Channel> channel)
    : stub_(ReplicaReplicaGrpc::NewStub(channel)) {}

int ReplicaReplicaGrpcClient::ReplicaPrePrepareClient(
    const string& msg, const string& sig, const string& client_msg) {
  // RestoreDataReq request;
  // RestoreDataReply reply;
  // ClientContext context;
  // std::chrono::time_point deadline =
  //     std::chrono::system_clock::now() + std::chrono::milliseconds(TIMEOUT);
  // context.set_deadline(deadline);
  // unique_ptr<ClientWriter<RestoreDataReq>> writer(
  //     stub_->RestoreData(&context, &reply));
  // for (int i : offset_v) {
  //   std::string buf = read(i);
  //   request.set_buf(buf);
  //   request.set_offset(i);
  //   if (!writer->Write(request)) {
  //     // Broken stream.
  //     break;
  //   }
  //   // crash testing code
  //   if (buf.find("crash_replica_replica_grpc_client_restore_data") !=
  //       string::npos) {
  //     cout << "Killing client process in write()\n";
  //     kill(getpid(), SIGABRT);
  //   }
  // }

  // writer->WritesDone();
  // Status status = writer->Finish();

  // if (status.ok()) {
  //   return reply.err();
  // }
  // cout << "There was an error in the server Write " << status.error_code()
  //      << endl;
  // return status.error_code();
}
int ReplicaReplicaGrpcClient::ReplicaPrepareClient(const string& msg,
                                                   const string& sig) {}
int ReplicaReplicaGrpcClient::ReplicaCommitClient(const string& msg,
                                                  const string& sig) {}
int ReplicaReplicaGrpcClient::ReplicaRelayRequestClient(const string& msg,
                                                        const string& sig) {}
// TODO: checkoint might remove for storing all logs
int ReplicaReplicaGrpcClient::ReplicaCheckpointClient(const string& msg,
                                                      const string& sig) {}