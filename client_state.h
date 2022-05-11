#ifndef CLIENT_STATE_H
#define CLIENT_STATE_H

#include <string>

template <typename T> class consumer_queue;

struct ClientState {
  std::unique_ptr<consumer_queue<std::pair<std::string, std::string>>> q;
};

#endif
