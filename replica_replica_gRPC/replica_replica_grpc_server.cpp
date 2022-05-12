#include "replica_replica_grpc_server.h"

#include <errno.h>
#include <fcntl.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <signal.h>
#include <unistd.h>

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "../common.h"
#include "replica_replica.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::StatusCode;

using namespace client_replica;
using namespace replica_replica;

ReplicaReplicaGrpcServiceImpl::ReplicaReplicaGrpcServiceImpl(
    ReplicaState* state)
    : state_(state) {}

Status ReplicaReplicaGrpcServiceImpl::PrePrepare(ServerContext* context,
                                                 const PrePrepareReq* request,
                                                 Empty* reply) {
  if (state_->replica_id == state_->primary) {
    std::cout
        << "The primary should not receive a pre-prepare message. Ignore it."
        << std::endl;
    return Status(StatusCode::PERMISSION_DENIED,
                  "Only the primary can send pre-prepare.");
  } else {
    const SignedMessage& preprepare = request->preprepare();
    const SignedMessage& client_message = request->client_message();

    // ==== Verify the authenticity of pre-prepare message ====

    PrePrepareCmd preprepare_cmd;
    if (!VerifyAndDecodeMessage(
            preprepare, state_->replicas_public_keys[state_->primary].get(),
            &preprepare_cmd)) {
      std::cout << "This pre-prepare is not signed properly by the primary."
                << std::endl;
      return Status(StatusCode::PERMISSION_DENIED,
                    "Only the primary can send pre-prepare.");
    }

    RequestCmd client_cmd;
    if (!client_cmd.ParseFromString(client_message.message())) {
      goto faulty_primary;
    }
    {
      RsaPtr rsa = CreateRsa(client_cmd.c(), true);
      if (!VerifyMessage(client_message.message(), client_message.signature(),
                         rsa.get())) {
        goto faulty_primary;
      }
    }

    if (preprepare_cmd.d() != Sha256Sum(client_message.message())) {
      goto faulty_primary;
    }

    if (preprepare_cmd.v() != state_->view) {
      goto faulty_primary;
    }

    // ==== Check n and add the request to history ====
    {
      // TODO: Check if n is between low and high water mark
      const std::lock_guard<std::mutex> lock(state_->operation_history_lock);
      if (preprepare_cmd.n() != state_->operation_history.size()) {
        goto faulty_primary;
      }
      state_->operation_history.emplace_back(client_cmd, preprepare_cmd.d());
    }
    state_->operation_history_cv.notify_all();

    // ==== Send prepare message ====

    for (const auto& replica_client : state_->replica_clients) {
      if (replica_client == nullptr) continue;
      if (replica_client->ReplicaPrepareClient(
              preprepare_cmd.v(), preprepare_cmd.n(), preprepare_cmd.d(),
              state_->replica_id) != 0) {
        // We can ignore the error because it does not need to take action for
        // a faulty replica.
        std::cout << "Fail to send prepare message to a replica. Ignore it."
                  << std::endl;
      }
    }
    return Status::OK;

  faulty_primary:
    // TODO: view change
    std::cout << "Faulty primary." << std::endl;
    return Status(StatusCode::PERMISSION_DENIED,
                  "Only the primary can send pre-prepare.");
  }
}

Status ReplicaReplicaGrpcServiceImpl::Prepare(ServerContext* context,
                                              const SignedMessage* request,
                                              Empty* reply) {
  // ==== Verify the authenticity of prepare message ====

  PrepareCmd preprepare_cmd;
  preprepare_cmd.ParseFromString(request->message());

  if (preprepare_cmd.i() >= state_->replicas_public_keys.size()) {
    std::cout << "Invalid replica number in the prepare message." << std::endl;
    return Status(StatusCode::PERMISSION_DENIED, "Invalid replica number");
  }

  if (preprepare_cmd.i() == state_->primary) {
    std::cout << "Prepare message from primary." << std::endl;
    return Status(StatusCode::PERMISSION_DENIED,
                  "Prepare message from primary");
  }

  if (!VerifyMessage(request->message(), request->signature(),
                     state_->replicas_public_keys[preprepare_cmd.i()].get())) {
    std::cout << "Incorrect signature of the prepare message." << std::endl;
    return Status(StatusCode::PERMISSION_DENIED, "Incorrect signature");
  }

  if (preprepare_cmd.v() != state_->view) {
    std::cout << "Incorrect view number in the prepare message." << std::endl;
    return Status(StatusCode::PERMISSION_DENIED, "Incorrect view number");
  }

  std::unique_lock<std::mutex> lock(state_->operation_history_lock);
  state_->operation_history_cv.wait(lock, [&]() {
    return preprepare_cmd.n() < state_->operation_history.size();
  });

  OperationState& op = state_->operation_history[preprepare_cmd.n()];
  if (preprepare_cmd.d() != op.digest) {
    std::cout << "Incorrect digest in the prepare message." << std::endl;
    return Status(StatusCode::PERMISSION_DENIED, "Incorrect digest");
  }

  // ==== Store the signatures ====

  op.prepare_signatures[preprepare_cmd.i()] = *request;

  // ==== Send commit message ====

  if (op.prepared(state_->f)) {
    lock.unlock();
    op.prepare_signatures_cv->notify_all();

    for (const auto& replica_client : state_->replica_clients) {
      if (replica_client == nullptr) continue;
      if (replica_client->ReplicaCommitClient(
              preprepare_cmd.v(), preprepare_cmd.n(), preprepare_cmd.d(),
              state_->replica_id) != 0) {
        // We can ignore the error because it does not need to take action for
        // a faulty replica.
        std::cout << "Fail to send commit message to a replica. Ignore it."
                  << std::endl;
      }
    }
  }

  return Status::OK;
}
Status ReplicaReplicaGrpcServiceImpl::Commit(ServerContext* context,
                                             const SignedMessage* request,
                                             Empty* reply) {
  // ==== Verify the authenticity of prepare message ====

  CommitCmd commit_cmd;
  commit_cmd.ParseFromString(request->message());

  if (commit_cmd.i() >= state_->replicas_public_keys.size()) {
    std::cout << "Invalid replica number in the commit message." << std::endl;
    return Status(StatusCode::PERMISSION_DENIED, "Invalid replica number");
  }

  if (!VerifyMessage(request->message(), request->signature(),
                     state_->replicas_public_keys[commit_cmd.i()].get())) {
    std::cout << "Incorrect signature of the commit message." << std::endl;
    return Status(StatusCode::PERMISSION_DENIED, "Incorrect signature");
  }

  if (commit_cmd.v() != state_->view) {
    std::cout << "Incorrect view number in the commit message." << std::endl;
    return Status(StatusCode::PERMISSION_DENIED, "Incorrect view number");
  }

  std::unique_lock<std::mutex> lock(state_->operation_history_lock);
  state_->operation_history_cv.wait(lock, [&]() {
    return commit_cmd.n() < state_->operation_history.size();
  });

  OperationState& op = state_->operation_history[commit_cmd.n()];

  if (commit_cmd.d() != op.digest) {
    std::cout << "Incorrect digest in the prepare message." << std::endl;
    return Status(StatusCode::PERMISSION_DENIED, "Incorrect digest");
  }

  op.prepare_signatures_cv->wait(lock,
                                 [&]() { return op.prepared(state_->f); });

  // ==== Store the signatures ====

  op.commit_signatures[commit_cmd.i()] = *request;

  // ==== Reply to the client ====

  {
    std::unique_lock<std::mutex> last_commit_lock(
        state_->last_commited_operation_lock);

    state_->last_commited_operation_cv.wait(last_commit_lock, [&]() {
      return state_->last_commited_operation == commit_cmd.n() - 1;
    });

    ReplyCmd reply_cmd;
    reply_cmd.set_v(state_->view);
    reply_cmd.set_t(op.request.t());
    reply_cmd.set_c(op.request.c());
    reply_cmd.set_i(state_->replica_id);

    if (PerformOperation(op.request.o(), reply_cmd.mutable_r()) != 0) {
      return Status(StatusCode::INTERNAL, "Failed to perform the operation");
    }

    state_->replies.do_fill(reply_cmd);

    state_->last_commited_operation = commit_cmd.n();
    state_->last_commited_operation_cv.notify_all();
  }

  lock.unlock();
  return Status::OK;
}

Status ReplicaReplicaGrpcServiceImpl::RelayRequest(ServerContext* context,
                                                   const SignedMessage* request,
                                                   Empty* reply) {
  return Status::OK;
}

Status ReplicaReplicaGrpcServiceImpl::Recover(
    ServerContext* context, const RecoverReq* request,
    ServerWriter<RecoverReply>* reply_writer) {
  int n = request->last_n();
  std::lock_guard<std::mutex> last_commit_lock(
      state_->last_commited_operation_lock);
  while (n < state_->last_commited_operation) {
    ++n;
    std::lock_guard<std::mutex> lock(state_->operation_history_lock);

    RecoverReply reply;
    for (auto [i, signed_message] :
         state_->operation_history[n].commit_signatures) {
      reply.add_signed_commits()->CopyFrom(signed_message);
    }
    reply_writer->Write(reply);
  }
  return Status::OK;
}

int ReplicaReplicaGrpcServiceImpl::PerformOperation(
    const OperationCmd& operation_cmd, ReplyData* result) {
  if (operation_cmd.has_read()) {
    const ReadRequestCmd& read = operation_cmd.read();

    std::string buf(kBlockSize, '\0');
    int res =
        pread(state_->mount_file_fd, buf.data(), kBlockSize, read.offset());
    if (res < 0) {
      perror("read");
      return res;
    } else if (res != kBlockSize) {
      return -1;
    }

    result->mutable_read()->set_data(buf);

    return 0;
  } else if (operation_cmd.has_write()) {
    const WriteRequestCmd& write = operation_cmd.write();

    if (write.data().size() != kBlockSize) {
      result->mutable_write()->set_ok(false);
      return 0;
    }

    int res;
    res = pwrite(state_->mount_file_fd, write.data().data(), kBlockSize,
                 write.offset());
    if (res < 0) {
      perror("pwrite");
      return res;
    }
    res = fsync(state_->mount_file_fd);
    if (res < 0) {
      perror("fsync");
      return res;
    }

    result->mutable_write()->set_ok(true);

    return 0;
  } else {
    return -1;
  }
}
