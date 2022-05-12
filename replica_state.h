#ifndef REPLICA_STATE_H
#define REPLICA_STATE_H

#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "client_replica.pb.h"
#include "consumer_queue.h"
#include "lib_crypto.h"

class ReplicaReplicaGrpcClient;
using common::SignedMessage;

struct OperationState {
  client_replica::RequestCmd request;
  std::string digest;
  std::unordered_map<int, SignedMessage> prepare_signatures;
  std::unordered_map<int, SignedMessage> commit_signatures;
  std::unique_ptr<std::condition_variable> prepare_signatures_cv;

  OperationState(const client_replica::RequestCmd& req, const std::string& d)
      : request(req),
        digest(d),
        prepare_signatures_cv(std::make_unique<std::condition_variable>()) {}

  bool prepared(int f, bool is_primary) {
    // 2*f+1 nodes agree on it, but the primary and itself do not send prepare
    // message. So, we only need 2*f-1 prepare messages.
    return (int)prepare_signatures.size() >= 2 * f + (is_primary ? 0 : -1);
  }
};
struct ReplicaState {
  int mount_file_fd;
  int replica_id;
  int primary;
  int view;
  int f;  // n >= 3f + 1
  std::vector<std::unique_ptr<ReplicaReplicaGrpcClient>> replica_clients;
  RsaPtr private_key = RsaPtr(nullptr, RSA_free);
  std::vector<RsaPtr> replicas_public_keys;
  ConsumerQueue<client_replica::ReplyCmd> replies;

  std::vector<OperationState> operation_history;
  std::mutex operation_history_lock;
  std::condition_variable operation_history_cv;

  int last_commited_operation = -1;
  std::mutex last_commited_operation_lock;
  std::condition_variable last_commited_operation_cv;
};

#endif
