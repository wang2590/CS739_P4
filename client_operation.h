#ifndef CLIENT_OPERATION_H
#define CLIENT_OPERATION_H
#include <string>
#include <vector>

#include "client_replica_gRPC/client_replica_grpc_client.h"
#include "client_state.h"
#include "consumer_queue.h"
class LibClient {
 public:
  LibClient(std::vector<std::string> ip_ports,
            const std::vector<RsaPtr>& replicas_public_keys, RsaPtr private_key,
            const std::string& public_key);
  void client_read(int offset);
  void client_write(int offset, std::string buf);

 private:
  ClientState state_;
  int quarum_num;
  std::vector<std::unique_ptr<ClientReplicaGrpcClient>> replicas;
};

#endif
