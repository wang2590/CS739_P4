#ifndef REPLICA_STATE_H
#define REPLICA_STATE_H

#include <memory>
#include <vector>

#include "replica_replica_gRPC/replica_replica_grpc_client.h"

struct ReplicaState {
  int mount_file_fd;
  std::vector<std::unique_ptr<ReplicaReplicaGrpcClient>> replica_clients;
};

#endif
