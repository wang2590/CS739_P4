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
  quarum_num = (ip_ports.size() - 1) / 3 * 2;
  for (std::string& ip_port : ip_ports) {
    replicas.push_back(std::make_unique<ClientReplicaGrpcClient>(grpc::CreateCustomChannel(
        ip_port, grpc::InsecureChannelCredentials(), ch_args), &state_));
  }
}

void LibClient::client_read(int offset) {
  std::string buf = "sdsd";
  std::string buf1 = "sdsd";
  std::vector<std::string> ret;
  auto timestamp = std::chrono::high_resolution_clock::now();

  // TODO:Signed the data

  int res = replicas[0]->clientRequest(buf1, buf);

  auto start_time = std::chrono::high_resolution_clock::now();
  // consumer
  while (ret.size() < this->quarum_num) {

    std::pair<std::string, std::string> res;
    int ret = state_.q->do_get(res, start_time);
    if (ret != 0) {
      return;
    }
      
    // TODO: Check the ret_res's timestamp
  }

  std::cout << "Read Success! Data: " << buf << " size = " << buf.size()
            << std::endl;

  return;
}

void LibClient::client_write(int offset, std::string buf) {
  auto timestamp = std::chrono::high_resolution_clock::now();
  std::string buf1 = "";

  // TODO: Signed the data
  // int res = this->replicas[0]->clientRequest(buf1, buf);
  int counter = 0;
  auto start_time = std::chrono::high_resolution_clock::now();

  // consumer
  while (counter < this->quarum_num) {
    
    std::pair<std::string, std::string> res;
    int ret = state_.q->do_get(res, start_time);
    if (ret != 0) {
      return;
    }
    counter++;
  }
  std::cout << "Write Success!\n";
}
