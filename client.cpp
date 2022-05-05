#include <iostream>
#include "client_operation.h"
#include "common.h"

void usage(char *argv[]) {
  printf("usage: %s -r <replica_ip_port>  [-h]\n", argv[0]);
}

int main(int argc, char *argv[]) {
  extern char *optarg;
  int opt;
  std::vector<std::string> replicas(4);
  int counter = 0;
  while ((opt = getopt(argc, argv, "p:b:h")) != -1) {
    switch (opt) {
      case 'p':
        replicas[counter++] = optarg;
        break;
      case 'h':
        usage(argv);
        exit(0);
      case '?':
        usage(argv);
        exit(1);
    }
    if (counter == 4) {
      break;
    }
  }
  if (replicas.size() < 4) {
    usage(argv);
    exit(1);
  }
  LibClient client = LibClient(replicas);

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