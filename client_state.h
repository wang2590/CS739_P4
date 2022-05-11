#ifndef CLIENT_STATE_H
#define CLIENT_STATE_H

#include <string>
#include "lib_crypto.h"

template <typename T> class consumer_queue;

struct ClientState {
  std::unique_ptr<consumer_queue<std::pair<std::string, std::string>>> q;
  RsaPtr public_key = RsaPtr(nullptr, RSA_free);
  std::vector<RsaPtr> replicas_public_keys;
  RsaPtr private_key = RsaPtr(nullptr, RSA_free);
};

#endif
