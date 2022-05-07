#include "client_operation.h"

#include <chrono>
#include <iomanip>
#include <sstream>

#include "common.h"
#include "lib_crypto.h"
// #include "consumer_queue.h"
int time_out = 100000;

LibClient::LibClient(std::vector<std::string> ip_ports) {
  state_.q = std::make_unique<consumer_queue>();
  grpc::ChannelArguments ch_args;
  quarum_num = (ip_ports.size() - 1) / 3;
  for (std::string& ip_port : ip_ports) {
    replicas.push_back(std::make_unique<ClientReplicaGrpcClient>(grpc::CreateCustomChannel(
        ip_port, grpc::InsecureChannelCredentials(), ch_args), &state_));
  }
}

void LibClient::client_read(int offset) {
  std::string dumb1 = "sdsd";
  std::string dumb2 = "sdsd";
  std::unordered_map<std::string, int> results; // message -> count
  auto timestamp = std::chrono::high_resolution_clock::now();

  // TODO:Signed the data

  int res = replicas[0]->clientRequest(dumb1, dumb2);

  auto start_time = std::chrono::high_resolution_clock::now();
  // consumer
  while (results.size() < 2 * this->quarum_num + 1) {
    std::pair<std::string, std::string> result;
    int ret = state_.q->do_get(start_time, result);
    if (ret != 0) {
      return;
    }

    // TODO: Check the message's timestamp
    


    // Timestamp match >> add to results, else discard

    // if count is over quarum_num break
    
  
  }

  std::cout << "Read Success! Data: " << buf << " size = " << buf.size()
            << std::endl;

  return;
}

void LibClient::client_write(int offset, std::string buf) {
  auto timestamp = std::chrono::high_resolution_clock::now();
  std::string dumb = "";
  std::set<int> s;
  // TODO: Signed the data


  int res = this->replicas[0]->clientRequest(dumb, buf);
  int counter = 0;
  auto start_time = std::chrono::high_resolution_clock::now();

  // consumer
  while (s.size() <= this->quarum_num) {
    std::pair<std::string, std::string> result;
    int ret = state_.q->do_get(start_time, result);
    if (ret != 0) {
      return;
    }
    // Check the timestamp


    
  }
  std::cout << "Write Success!\n";
}
