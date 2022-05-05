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

ClientReplicaGprcClient::ClientReplicaGrpcClient(
    std::shared_ptr<Channel> channel)
    : stub_(ClientReplicaGprc::NewStub(channel)) {}

int ClientReplicaGrpcClient::clientReadBlock(const string& msg,
                                             const string& sig) {
  // SignedMessage request;
  // request.set_message(msg);
  // request.set_signature(sig);

  // Empty reply;
  // ClientContext context;
  // chrono::time_point deadline =
  //     chrono::system_clock::now() + chrono::milliseconds(TIMEOUT);
  // // context.set_deadline(deadline);
  // // Status status = stub_->ReadBlock(&context, request, &reply);

  // // // testing code
  // // if (reply.buf().find("crash_client_server_grpc_client_read") !=
  // //     string::npos) {
  // //   cout << "Killing client process in read()\n";
  // //   kill(getpid(), SIGABRT);
  // }

  // // cout << "Client received read: " << reply.buf() << endl;
  // buf.append(reply.buf());

  // if (status.ok()) {
  //   if (reply.err()) {
  //     // cout << "There was an error in the server Read " << reply.err() <<
  //     // endl;
  //     return reply.err();
  //   }
  //   return 0;
  // }
  // // cout << "There was an error in the server Read " <<
  // status.error_message()
  // //<< endl;
  // return status.error_code();
}

int ClientReplicagRPCClient::clientWriteBlock(const int& offset,
                                              const string& buf) {
  // cout << "client server grpc client write" << endl;
  // WriteBlockReq request;
  // WriteBlockReply reply;
  // ClientContext context;
  // chrono::time_point deadline =
  //     chrono::system_clock::now() + chrono::milliseconds(TIMEOUT);
  // context.set_deadline(deadline);
  // request.set_buf(buf);
  // request.set_offset(offset);
  // Status status = stub_->WriteBlock(&context, request, &reply);

  // // crash testing code
  // if (buf.find("crash_client_server_grpc_client_write") != string::npos) {
  //   cout << "Killing client process in write()\n";
  //   kill(getpid(), SIGABRT);
  // }

  // if (status.ok()) {
  //   return reply.err();
  // }
  // cout << "There was an error in the server Write " << status.error_code()
  //      << endl;
  // return status.error_code();
}
