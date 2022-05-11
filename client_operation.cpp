#include "client_operation.h"

#include <chrono>
#include <iomanip>
#include <set>
#include <sstream>
#include <unordered_map>
#include <utility>

#include "common.h"
#include "consumer_queue.h"
using namespace std::chrono_literals;
using namespace client_replica;
typedef std::pair<std::string, std::string> p;

LibClient::LibClient(std::vector<std::string>& ip_ports,
                     std::vector<std::string>& replicas_public_keys,
                     std::string private_key, std::string public_key) {
  state_.q = std::make_unique<consumer_queue<p>>();
  state_.private_key = CreateRsaWithFilename(private_key, false);
  state_.public_key = CreateRsaWithFilename(public_key, true);
  for (auto keys : replicas_public_keys) {
    state_.replicas_public_keys.push_back(CreateRsaWithFilename(keys, true));
  }
  quarum_num = (ip_ports.size() - 1) / 3;
  for (std::string& ip_port : ip_ports) {
    auto client = std::make_unique<ClientReplicaGrpcClient>(
        grpc::CreateChannel(ip_port, grpc::InsecureChannelCredentials()),
        &state_);
    this->replicas.push_back(std::move(client));
  }
}

void LibClient::client_read(int offset) {
  using namespace std::literals;
  std::unordered_map<std::string, int> hashTable;  // message -> count
  auto timestamp = std::chrono::high_resolution_clock::now();

  
  ReadRequestCmd read_cmd;
  OperationCmd op;
  RequestCmd cmd;
  read_cmd.set_offset(offset);
  op.set_read(read_cmd);
  cmd.set_o(op);
  cmd.set_t(timestamp);
  cmd.set_c(state_.public_key.get());
  int res = replicas[0]->clientRequest(cmd);

  auto time_out = std::chrono::system_clock::now() + 100ms;
  // consumer
  while (hashTable.size() < 2 * this->quarum_num + 1) {
    std::pair<std::string, std::string> result;
    int ret = state_.q->do_get(time_out, result);
    if (ret != 0) {
      return;
    }
    
    // TODO: Check the message's timestamp

    // Timestamp match >> add to hashTable, else discard

    // if count is over quarum_num, read success
  }

  std::cout << "Read Success! Data: " << dumb2 << " size = " << dumb2.size()
            << std::endl;

  return;
}

void LibClient::client_write(int offset, std::string buf) {
  auto timestamp = std::chrono::high_resolution_clock::now();
  std::string dumb = "";
  std::set<int> s;  // Replica Id
  int counter = 0;

  RequestCmd cmd;
  WriteRequestCmd write_cmd;
  OperationCmd op;
  write_cmd.set_offset(offset);
  write_cmd.set_data(buf);
  op.set_write(write_cmd);
  cmd.set_o(op);
  cmd.set_t(timestamp);
  cmd.set_c(state_.public_key.get());
  int res = replicas[0]->clientRequest(cmd);

  auto time_out = timestamp + 100ms;

  // consumer
  while (s.size() < 2 * this->quarum_num + 1) {
    std::pair<std::string, std::string> result;
    int ret = state_.q->do_get(time_out, result);
    if (ret == -1) {
      break;
    }

    // Verify Message

    // TODO: Check the message's timestamp

    // Timestamp match >> add to hashTable, else discard

    // if count is over quarum_num, write success!
    if (counter > this->quarum_num) {
      std::cout << "Write Success!\n";
      return;
    }
  }
  std::cout << "Write Failed!\n";
}
