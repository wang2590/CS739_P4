#ifndef CLIENT_OPERATION_H
#define CLIENT_OPERATION_H
#include <string>
#include <vector>

#include "client_replica_gRPC/client_replica_grpc_client.h"

class LibClient {
 public:
  LibClient(std::vector<std::string> ip_ports);
  void client_read(int offset);
  void client_write(int offset, std::string buf);

 private:
  int quarum_num;
  std::vector<ClientReplicaGrpc*> replicas;
};

#endif
