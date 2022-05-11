#ifndef REPLICA_STATE_H
#define REPLICA_STATE_H

#include <memory>
#include <string>
#include <vector>

#include "client_replica.pb.h"
#include "consumer_queue.h"
#include "lib_crypto.h"

class ReplicaReplicaGrpcClient;

struct ReplicaState {
  int mount_file_fd;
  int replica_id;
  int primary;
  int view;
  int f;  // n >= 3f + 1
  std::vector<std::unique_ptr<ReplicaReplicaGrpcClient>> replica_clients;
  RsaPtr private_key = RsaPtr(nullptr, RSA_free);
  std::vector<RsaPtr> replicas_public_keys;
  consumer_queue<client_replica::ReplyCmd> replies;
};

#endif
