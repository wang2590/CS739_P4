#include "client_operation.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include "lib_crypto.h"
#include "common.h"
int time_out = 100000;

void LibClient::client_read(int offset) {
  std::string buf = "";
  std::string buf1 = "";
  std::vector<std::string> ret;
  auto timestamp = std::chrono::high_resolution_clock::now();

  // TODO:Signed the data

  int res = this->replicas[0]->clientRequest(buf1, buf);

  auto start_time = std::chrono::high_resolution_clock::now();
  // consumer
  while (ret.size() < this->quarum_num) {
    while (q.consumer_ready()) {

      // Check timeout
      auto end_time = std::chrono::high_resolution_clock::now();
      if (std::chrono::duration_cast<std::chrono::microseconds>(end_time -
                                                                      start_time)
                    .count() >= time_out) {
        std::cout << "Timeout: Read Failed!\n";
        return;
      }
    }

    // Check time out
    auto end_time = std::chrono::high_resolution_clock::now();
    if (std::chrono::duration_cast<std::chrono::microseconds>(end_time -
                                                                    start_time)
                  .count() >= time_out) {
      std::cout << "Timeout: Read Failed!\n";
      return;
    }


    std::unique_lock<std::mutex> ul(q.lock);
    std::string ret_res = q.do_get();
    ul.unlock();
    
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
  int res = this->replicas[0]->clientRequest(buf1, buf);
  int counter = 0;
  auto start_time = std::chrono::high_resolution_clock::now();

  // consumer
  while (counter < this->quarum_num) {
    while (q.consumer_ready()) {
      auto end_time = std::chrono::high_resolution_clock::now();
      if (std::chrono::duration_cast<std::chrono::microseconds>(end_time -
                                                                      start_time)
                    .count() > time_out) {
        std::cout << "Timeout: Write Failed!\n";
        return;
      }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    if (std::chrono::duration_cast<std::chrono::microseconds>(end_time -
                                                                    start_time)
                  .count() > time_out) {
      std::cout << "Timeout: Write Failed!\n";
      return;
    }

    std::unique_lock<std::mutex> ul(q.lock);
    std::string ret_res = q.do_get();
    ul.unlock();

    counter++;
  }
  std::cout << "Write Success!\n";
  
}

LibClient::LibClient(std::vector<std::string> ip_ports) {
  grpc::ChannelArguments ch_args;
  this->quarum_num = (ip_ports.size() - 1) / 3;
  for (std::string ip_port : ip_ports) {
    this->replicas.push_back(new ClientReplicaGrpcClient(grpc::CreateCustomChannel(
      ip_port, grpc::InsecureChannelCredentials(), ch_args)));
  }
}