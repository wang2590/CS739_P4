#ifndef CLIENT_OPERATION_H
#define CLIENT_OPERATION_H
#include <string>
#include <vector>
#include "client_replica_gRPC/client_replica_grpc_client.h"
#include "consumer_queue.h"

class LibClient {
 public:
  LibClient(std::vector<std::string> ip_ports);
  void client_read(int offset);
  void client_write(int offset, std::string buf);

 private:
  consumer_queue<std::string> q;
  int quarum_num;
  std::vector<ClientReplicaGrpcClient*> replicas;
  ClientReplicaGrpcClient* the_replica;
};

#endif
