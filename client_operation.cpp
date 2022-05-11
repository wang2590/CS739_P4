#include "client_operation.h"

#include <chrono>
#include <iomanip>
#include <set>
#include <sstream>
#include <unordered_map>
#include <utility>

#include "client_replica.grpc.pb.h"
#include "common.h"
#include "consumer_queue.h"

using namespace std::chrono_literals;
using namespace client_replica;
using namespace std;

LibClient::LibClient(const std::vector<std::string>& ip_ports,
                     const std::vector<std::string>& replicas_public_keys,
                     const std::string& private_key_path,
                     const std::string& public_key_path) {
  state_.q = std::make_unique<consumer_queue<ReplyCmd>>();
  state_.private_key = CreateRsaWithFilename(private_key_path, false);
  state_.public_key = readFile(public_key_path);
  for (auto keys : replicas_public_keys) {
    state_.replicas_public_keys.push_back(CreateRsaWithFilename(keys, true));
  }
  int index = 0;
  quarum_num = (ip_ports.size() - 1) / 3;
  for (const std::string& ip_port : ip_ports) {
    auto client = std::make_unique<ClientReplicaGrpcClient>(
        grpc::CreateChannel(ip_port, grpc::InsecureChannelCredentials()),
        &state_, index++);
    this->replicas.push_back(std::move(client));
  }
  // create thread to call clientReply from each replicas
  initClientReplyThread();
}
void LibClient ::initClientReplyThread() {
  for (auto& i : replicas) {
    std::thread t = i->thread_func();
  }
  cout << "Start clientReply threads Call" << endl;
}

std::string LibClient::readFile(std::string input) {
  fstream newfile;
  newfile.open(input, ios::in);
  string output;
  if (newfile.is_open()) {
    string tp;
    while (getline(newfile, tp)) {
      output += tp;
    }
    newfile.close();  // close the file object.
  }
  return output;
}

void LibClient::client_read(int offset) {
  using namespace std::literals;
  std::unordered_map<std::string, int> hashTable;  // message -> count
  std::unordered_set<int> verifyID;
  // set double timestamp
  auto timestamp = std::chrono::high_resolution_clock::now();
  auto timestamp_ns =
      std::chrono::time_point_cast<std::chrono::nanoseconds>(timestamp);
  double timestamp_d = timestamp_ns.time_since_epoch().count();

  RequestCmd cmd;
  cmd.mutable_o()->mutable_read()->set_offset(offset);
  cmd.set_t(timestamp_d);
  cmd.set_c(state_.public_key);
  int res = replicas[0]->clientRequest(cmd);

  auto time_out = std::chrono::system_clock::now() + 1000ms;
  // consumer
  while (hashTable.size() < 2 * quarum_num + 1) {
    ReplyCmd result;
    int ret = state_.q->do_get(time_out, result);
    if (ret != 0) {
      cout << "read command timeout error" << endl;
      return;
    }
    if (result.t() == timestamp_d &&
        verifyID.find(result.i()) == verifyID.end()) {
      std::string data = result.r().read().data();
      hashTable[data]++;
      verifyID.insert(result.i());
      if (hashTable[data] > quarum_num) {
        cout << "read success: " << data << endl;
      }
    }
  }
  cout << "read Failed" << endl;
  return;
}
void LibClient::client_write(int offset, std::string buf) {
  // set double timestamp
  auto timestamp = std::chrono::high_resolution_clock::now();
  auto timestamp_ns =
      std::chrono::time_point_cast<std::chrono::nanoseconds>(timestamp);
  double timestamp_d = timestamp_ns.time_since_epoch().count();
  std::string dumb = "";
  std::set<int> s;  // Replica Id
  int counter = 0;

  RequestCmd cmd;

  cmd.mutable_o()->mutable_write()->set_offset(offset);
  cmd.mutable_o()->mutable_write()->set_data(buf);
  cmd.set_t(timestamp_d);
  // TODO: add public key
  cmd.set_c(state_.public_key);
  int res = replicas[0]->clientRequest(cmd);

  auto time_out = timestamp + 100ms;

  // consumer
  while (s.size() < 2 * this->quarum_num + 1) {
    ReplyCmd result;
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
