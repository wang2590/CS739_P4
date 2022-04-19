#include <iostream>

#include "client_operation.h"
#include "common.h"

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

  client_init(primary_ip_port, backup_ip_port);

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
      client_read(offset);
    } else if (action == "write") {
      if (!(std::cin >> offset >> data)) {
        std::cout << "Wrong input\n";
        continue;
      }
      data.resize(kBlockSize);
      client_write(offset, data);
    } else if (action == "quit") {
      break;
    } else {
      std::cout << "Wrong action\n";
      continue;
    }
  }
  return 0;
}