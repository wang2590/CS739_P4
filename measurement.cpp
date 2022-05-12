#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <istream>
#include <ostream>
#include <thread>
#include <vector>
#include <nlohmann/json.hpp>
#include "client_operation.h"
#include "common.h"
using nlohmann::json;

#define MB 1024 * 1024
#define KB 1024

const std::string perf_data = "./perf_data";
const std::string perf_stat = "./perf_stat";
namespace fs = std::filesystem;

std::unique_ptr<LibClient> client;

vector<int> file_sizes = {1, 8, 16, 32, 64, 128, 256, 512, 1024};

char *getBuffer(long bytes) {
  char *buff = (char *)malloc(bytes + 1);
  memset(buff, 'a', bytes);
  buff[bytes] = '\0';
  return buff;
}

void write_correctness_test(std::ostream &output) {
  output << "write_correctness test\n";
  std::vector<std::string> tests = {"write_test1", "write_test2", "write_test3",
                                    "write_test4"};
  bool pass = true;
  for (int i = 0; i < tests.size(); ++i) {
    tests[i].resize(kBlockSize);
    int offset = (i + 1) * kBlockSize;
    client->client_write(offset, tests[i]);
    std::string result = client->client_read(offset);
    if (result.compare(tests[i])) {
      output << "Primary not correct\n";
      output << i << ": " << tests[i] << "   " << result << "\n";
      pass = false;
    }
  }


  if (pass) output << "Passed\n";
  output << "\n";

}




void write_latency_test() {
  ifstream file;
  std::ofstream outfile(perf_stat + "/write_latency.txt");
  outfile << "PBFT(Microsecond):\n";
  std::string filename = perf_data + "/4_KB";
  file.open(filename);
  int offset = 0;
  std::string str;
  if (file.is_open()) {
    auto duration = 0;
    while (file.good()) {
      file >> str;

      for (int i = 0; i < 10; ++i) {
        auto start_time = std::chrono::high_resolution_clock::now();
        client->client_write(offset, str);
        auto end_time = std::chrono::high_resolution_clock::now();

        duration = std::chrono::duration_cast<std::chrono::microseconds>(
                      end_time - start_time)
                      .count();
        outfile << duration << "\n"; 
      }
        
      offset += str.size();
    }
    
  }
  file.close();   
  outfile.close();

}

void read_latency_test(bool unaligned = false) {
  std::ofstream outfile(perf_stat + "/" + (unaligned ? "unaligned_" : "") +
                        "read_latency.txt");

  outfile << "Time in microsecond"
          << "\n";
  auto duration = 0;
  for (long int i = MB + unaligned; i <= 1024 * MB; i *= 2) {
    auto start_time = std::chrono::high_resolution_clock::now();
    client->client_read(i);
    auto end_time = std::chrono::high_resolution_clock::now();
    outfile << std::chrono::duration_cast<std::chrono::microseconds>(end_time -
                                                                     start_time)
                   .count()
            << "\n";
    duration += std::chrono::duration_cast<std::chrono::microseconds>(
                    end_time - start_time)
                    .count();
  }
  outfile << "average = " << duration / 10 << " microsecond\n";
  outfile.close();
}


void testing_init() {
  fs::remove_all(perf_data);
  fs::create_directory(perf_data);
  fs::remove_all(perf_stat);
  fs::create_directory(perf_stat);

  std::string filename = perf_data + "/4_KB";
  char *buffer = getBuffer(4 * KB);
  std::ofstream file(filename);
  file.write(buffer, 4 * KB);
  file.close();
}

void correctness_test() {
  std::ofstream file(perf_stat + "/correctness_test");
  file << "---------------------------------\n";
  write_correctness_test(file);
  file << "---------------------------------\n";
  file.close();
}

void performance_test() {
  write_latency_test();
  read_latency_test();
  read_latency_test(true);
}

void usage(char *argv[]) {
  printf("usage: %s -c config file [-h]\n", argv[0]);
}

int main(int argc, char *argv[]) {
  extern char *optarg;
  int opt;
  std::string config_file;
  int counter = 0;
  while ((opt = getopt(argc, argv, "c:h")) != -1) {
    switch (opt) {
      case 'c':
        config_file = optarg;
        break;
      case 'h':
        usage(argv);
        exit(0);
      case '?':
        usage(argv);
        exit(1);
    }
  }
  if (config_file == "") {
    usage(argv);
    exit(1);
  }

  std::ifstream config_file_stream(config_file);
  json config = json::parse(config_file_stream);

  // Set up the connection
  std::vector<std::string> replicas_ip_ports;
  std::vector<std::string> replicas_public_keys;
  for (auto &replica_conf : config["replicas"]) {
    replicas_ip_ports.push_back(replica_conf["ip_port"]);
    replicas_public_keys.push_back(replica_conf["public_key_path"]);
  }
  client =
      std::make_unique<LibClient>(replicas_ip_ports, replicas_public_keys,
                config["private_key_path"], config["public_key_path"]);
  testing_init();

  std::cout << "Start correctness test\n";
  correctness_test();
  std::cout << "Start performance test\n";
  performance_test();
}