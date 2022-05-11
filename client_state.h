#ifndef CLIENT_STATE_H
#define CLIENT_STATE_H

#include <string>

#include "client_replica.grpc.pb.h"
#include "lib_crypto.h"

template <typename T>
class consumer_queue;

struct ClientState {
  std::unique_ptr<consumer_queue<client_replica::ReplyCmd>> q;
  std::string public_key;
  std::vector<RsaPtr> replicas_public_keys;
  RsaPtr private_key = RsaPtr(nullptr, RSA_free);
};

#endif
