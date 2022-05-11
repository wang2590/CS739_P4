#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

#include "client_operation.h"
#include "common.h"
using nlohmann::json;

void usage(char *argv[]) { printf("usage: %s -c config_file [-h]\n", argv[0]); }

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
  LibClient client =
      LibClient(replicas_ip_ports, replicas_public_keys,
                config["private_key_path"], config["public_key_path"]);

  std::string action = "";
  int offset = -1;
  std::string data = "";
  int res = 0;

  while (std::cin >> action) {
    if (action == "read") {
      if (!(std::cin >> offset)) {
        std::cout << "Wrong input \n";
        continue;
      }
      client.client_read(offset);
    } else if (action == "write") {
      if (!(std::cin >> offset >> data)) {
        std::cout << "Wrong input\n";
        continue;
      }
      data.resize(kBlockSize);
      client.client_write(offset, data);
    } else if (action == "quit") {
      break;
    } else {
      std::cout << "Wrong action\n";
      continue;
    }
  }
  return 0;
}