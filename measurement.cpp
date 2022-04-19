#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <istream>
#include <ostream>
#include <thread>
#include <vector>

#include "client_operation.h"
#include "common.h"

#define MB 1024 * 1024
#define KB 1024

const std::string perf_data = "./perf_data";
const std::string perf_stat = "./perf_stat";
namespace fs = std::filesystem;

extern ClientServergRPCClient *curr;
extern ClientServergRPCClient *primary_client;
extern ClientServergRPCClient *backup_client;

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
    client_write(offset, tests[i]);
    std::string result = client_read(offset);
    if (result.compare(tests[i])) {
      output << "Primary not correct\n";
      output << i << ": " << tests[i] << "   " << result << "\n";
      pass = false;
    }
  }
  // Crash primary to test backup correctness
  primary_client->clientWriteBlock(0, "crash_grpc_server_write");

  // Test backup
  for (int i = 0; i < tests.size(); ++i) {
    int offset = (i + 1) * kBlockSize;
    std::string result = client_read(offset);
    if (result.compare(tests[i])) {
      output << "Backup not correct\n";
      output << i << ": " << tests[i] << "   " << result << "\n";
      pass = false;
    }
  }
  if (pass) output << "Passed\n";
  output << "\n";

  std::cout << "Please Reboot primary\n";
  sleep(20);
}

void primary_resurrection_correctness_test(std::ostream &output) {
  // Crash primary to test backup correctness
  primary_client->clientWriteBlock(0, "crash_grpc_server_write");
  bool pass = true;
  output << "Primary resurrection correctness Test\n";

  // write to backup
  std::vector<std::string> tests = {
      "primary_resurrection1", "primary_resurrection2", "primary_resurrection3",
      "primary_resurrection4"};
  for (int i = 0; i < tests.size(); ++i) {
    tests[i].resize(kBlockSize);
    int offset = (i + 1) * kBlockSize;
    client_write(offset, tests[i]);
  }
  if (curr != backup_client) {
    output << "failed: Connect to primary\n";
    pass = false;
    return;
  }
  std::string buf;
  std::cout << "Please Reboot primary\n";

  // Wait until sync over
  sleep(20);

  // Check the data.
  for (int i = 0; i < tests.size(); ++i) {
    int offset = (i + 1) * kBlockSize;
    std::string result = client_read(offset);
    if (result.compare(tests[i])) {
      pass = false;
      output << "Primary not correct\n";
      output << i << ": " << tests[i] << "   " << result << "\n";
    }
  }
  if (pass) output << "Passed\n";
  output << "\n";
}

void backup_resurrection_correctness_test(std::ostream &output) {
  output << "Backup resurrection correctness Test\n";
  // send crash info to backup to make it crashed
  backup_client->clientWriteBlock(0, "crash_grpc_server_write");
  bool pass = true;
  // write to primary
  std::vector<std::string> tests = {
      "backup_resurrection1", "backup_resurrection2", "backup_resurrection3",
      "backup_resurrection4"};
  for (int i = 0; i < tests.size(); ++i) {
    tests[i].resize(kBlockSize);
    int offset = (i + 1) * kBlockSize;
    client_write(offset, tests[i]);
  }

  std::string buf;
  std::cout << "Please Reboot backup\n";

  sleep(20);

  // send crash info to primary to make it crashed
  primary_client->clientWriteBlock(0, "crash_grpc_server_write");

  // Check the data.
  for (int i = 0; i < tests.size(); ++i) {
    int offset = (i + 1) * kBlockSize;
    std::string result = client_read(offset);
    if (result.compare(tests[i])) {
      output << "Backup not correct\n";
      output << i << ": " << tests[i] << "   " << result << "\n";
      pass = false;
    }
  }
  if (curr == primary_client) {
    output << "failed: Connect to Backup\n";
    pass = false;
  }
  if (pass) output << "Passed\n";
  output << "\n";

  std::cout << "Please Reboot Primary\n";
}

void crash_write_latency_test() {
  ifstream file;
  std::ofstream outfile(perf_stat + "/crash_write_latency.txt");
  std::string filename = perf_data + "/4_KB";
  file.open(filename);
  int offset = 0;
  std::string str;

  primary_client->clientWriteBlock(0, "crash_grpc_server_write");

  if (file.is_open()) {
    auto duration = 0;
    while (file.good()) {
      file >> str;

      auto start_time = std::chrono::high_resolution_clock::now();
      client_write(offset, str);
      auto end_time = std::chrono::high_resolution_clock::now();

      duration = std::chrono::duration_cast<std::chrono::microseconds>(
                     end_time - start_time)
                     .count();
      offset += str.size();
    }
    outfile << duration << " microsecond\n";
  }
  file.close();
  std::cout << "Please Reboot Primary\n";
  sleep(10);
}

void write_latency_test() {
  ifstream file;
  std::ofstream outfile(perf_stat + "/write_latency.txt");
  outfile << "Primary_backup(Microsecond):\n";
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
        client_write(offset, str);
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

  // Single server
  backup_client->clientWriteBlock(0, "crash_grpc_server_write");

  outfile << "Single Server(Microsecond):\n";
  
  for (int i = 0; i < 10; ++i) {
    auto start_time = std::chrono::high_resolution_clock::now();
    client_write(offset, str);
    auto end_time = std::chrono::high_resolution_clock::now();

    outfile << std::chrono::duration_cast<std::chrono::microseconds>(end_time -
                                                                    start_time)
                  .count()
            << "\n";
  }

    
  outfile.close();
  std::cout << "Please Reboot backup\n";
  sleep(20);
}

void read_latency_test(bool unaligned = false) {
  std::ofstream outfile(perf_stat + "/" + (unaligned ? "unaligned_" : "") +
                        "read_latency.txt");

  outfile << "Time in microsecond"
          << "\n";
  auto duration = 0;
  for (long int i = MB + unaligned; i <= 1024 * MB; i *= 2) {
    auto start_time = std::chrono::high_resolution_clock::now();
    client_read(i);
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

void recovery_time() {
  primary_client->clientWriteBlock(0, "crash_grpc_server_write");
  std::cout << "Please Rebbot Primary\n";

  std::string buf;
  auto start_time = std::chrono::high_resolution_clock::now();
  while (primary_client->clientReadBlock(0, buf))
    ;
  auto end_time = std::chrono::high_resolution_clock::now();

  ofstream outfile(perf_stat + "/recovery_latency");
  outfile << "Primary recovery time: "
          << std::chrono::duration_cast<std::chrono::microseconds>(end_time -
                                                                   start_time)
                 .count()
          << " microsecond\n";

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
  primary_resurrection_correctness_test(file);
  file << "---------------------------------\n";
  backup_resurrection_correctness_test(file);
  file << "---------------------------------\n";
  file.close();
}

void performance_test() {
  write_latency_test();
  crash_write_latency_test();
  read_latency_test();
  read_latency_test(true);
  recovery_time();
}

void usage(char *argv[]) {
  printf("usage: %s -p primary_ip_port -b backup_ip_port [-h]\n", argv[0]);
}

int main(int argc, char *argv[]) {
  extern char *optarg;
  int opt;
  std::string primary_ip_port;
  std::string backup_ip_port;
  while ((opt = getopt(argc, argv, "p:b:h")) != -1) {
    switch (opt) {
      case 'p':
        primary_ip_port = optarg;
        break;
      case 'b':
        backup_ip_port = optarg;
        break;
      case 'h':
        usage(argv);
        exit(0);
      case '?':
        usage(argv);
        exit(1);
    }
  }
  if (primary_ip_port == "" || backup_ip_port == "") {
    usage(argv);
    exit(1);
  }
  testing_init();
  client_init(primary_ip_port, backup_ip_port);
  std::cout << "Start correctness test\n";
  correctness_test();
  sleep(10);
  std::cout << "Start performance test\n";
  performance_test();
}