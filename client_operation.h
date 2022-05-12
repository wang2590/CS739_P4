#ifndef CLIENT_OPERATION_H
#define CLIENT_OPERATION_H
#include <fstream>
#include <string>
#include <thread>
#include <vector>

#include "client_replica_gRPC/client_replica_grpc_client.h"
#include "client_state.h"
#include "consumer_queue.h"
#include "lib_crypto.h"

class LibClient {
 public:
  LibClient(const std::vector<std::string>& ip_ports,
            const std::vector<std::string>& replicas_public_keys,
            const std::string& private_key_path,
            const std::string& public_key_path);
  std::string client_read(int offset);
  void client_write(int offset, std::string buf);

 private:
  ClientState state_;
  int quarum_num;
  std::vector<std::unique_ptr<ClientReplicaGrpcClient>> replicas;
  void initClientReplyThread();
  std::string readFile(std::string input);
  std::vector<std::thread> client_reply_threads_;
};

#endif
