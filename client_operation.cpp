#include "client_operation.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <utility>
#include <unordered_map>
#include <set>
#include "consumer_queue.h"
#include "common.h"
using namespace std::chrono_literals;
typedef std::pair<std::string, std::string> p;
 
LibClient::LibClient(std::vector<std::string> ip_ports) {
  state_.q = std::make_unique<consumer_queue<p>>();
  //std::unique_ptr<consumer_queue<int>> test = std::make_unique<consumer_queue<int>>();
  quarum_num = (ip_ports.size() - 1) / 3;
  for (std::string& ip_port : ip_ports) {
    auto client = std::make_unique<ClientReplicaGrpcClient>(
          grpc::CreateChannel(ip_port,
                              grpc::InsecureChannelCredentials()),
          &state_);
    this->replicas.push_back(std::move(client));
  }
}

void LibClient::client_read(int offset) {
  using namespace std::literals;
  std::string dumb1 = "sdsd";
  std::string dumb2 = "sdsd";
  std::unordered_map<std::string, int> hashTable; // message -> count
  auto timestamp = std::chrono::high_resolution_clock::now();
  
  // TODO:Signed the data

  int res = replicas[0]->clientRequest(dumb1, dumb2);

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
  std::set<int> s; // Replica Id
  int counter = 0;
  // TODO: Signed the data


  int res = this->replicas[0]->clientRequest(dumb, buf);
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
