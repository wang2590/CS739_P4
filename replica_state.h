#ifndef REPLICA_STATE_H
#define REPLICA_STATE_H

#include <memory>
#include <string>
#include <vector>

class ReplicaReplicaGrpcClient;

struct ReplicaState {
  int mount_file_fd;
  std::vector<std::unique_ptr<ReplicaReplicaGrpcClient>> replica_clients;
  std::string private_key_path;
  std::vector<std::string> replicas_public_key_paths;
};

#endif
