#include "client_operation.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include "common.h"
int time_out = 100000;

void LibClient::client_read(int offset) {
  std::string buf = "";
  std::vector<std::string> res;
  auto timestamp = std::chrono::high_resolution_clock::now();

  // TODO:Signed the data

  int res = this->replicas[0]->clientReadBlock(offset, buf);

  auto start_time = std::chrono::high_resolution_clock::now();
  // consumer
  while (res.size() < this->quarum_num) {
    // TODO: Check the lock
    while () {
      auto end_time = std::chrono::high_resolution_clock::now();
      if (std::chrono::duration_cast<std::chrono::microseconds>(end_time -
                                                                      start_time)
                    .count() >= 100000) {
        std::cout << "Timeout: Read Failed!\n";
        return "";
      }
    }
    
    // TODO: Grab lock and fetch the data

  }

  std::cout << "Read Success! Data: " << buf << " size = " << buf.size()
          << std::endl;
 

  return buf;
}

void LibClient::client_write(int offset, std::string buf) {
  auto timestamp = std::chrono::high_resolution_clock::now();
  // TODO: Signed the data
  int res = curr->clientWriteBlock(offset, buf);

  auto start_time = std::chrono::high_resolution_clock::now();
  int counter = 0;

  // consumer
  while (counter < this->quarum_num) {
    // TODO: Check the lock
    while () {
      auto end_time = std::chrono::high_resolution_clock::now();
      if (std::chrono::duration_cast<std::chrono::microseconds>(end_time -
                                                                      start_time)
                    .count() > time_out) {
        std::cout << "Timeout: Write Failed!\n";
        return;
      }
    }
    // TODO: Grab lock and fetch data
    counter++;
  }
  std::cout << "Write Success!\n";
  
}

LibClient::LibClient(std::vector<std::string> ip_ports) {
  grpc::ChannelArguments ch_args;
  this->quarum_num = (ip_ports.size() - 1) / 3;
  for (std::string ip_port : ip_ports) {
    this->replicas.push_back(new ClientServergRPCClient(grpc::CreateCustomChannel(
      ip_port, grpc::InsecureChannelCredentials(), ch_args))));
  }
}