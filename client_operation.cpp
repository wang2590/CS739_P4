#include "client_operation.h"

#include <iomanip>
#include <sstream>
#include "common.h"

ClientServergRPCClient* curr;
ClientServergRPCClient* primary_client;
ClientServergRPCClient* backup_client;

std::string bytes_to_hex(std::string bytes) {
  std::stringstream ss;
  ss << std::hex;

  for (int i = 0; i < bytes.size(); ++i)
    ss << std::setw(2) << std::setfill('0') << (int)bytes[i];

  return ss.str();
}

std::string client_read(int offset) {
  std::string buf = "";
  int res = curr->clientReadBlock(offset, buf);

  while (res) {
    if (curr == primary_client) {
      //std::cout << "Primary Crashed\n";
      curr = backup_client;
    } else if (curr == backup_client) {
      //std::cout << "Backup Blocked\n";
      curr = primary_client;
    }
    res = curr->clientReadBlock(offset, buf);
  }

  std::cout << "Read Success! Data: " << buf << " size = " << buf.size()
            << std::endl;

  return buf;
}

void client_write(int offset, std::string buf) {
  std::cout << "Data: " << buf << " size = " << buf.size() << std::endl;
  int res = 0;
  res = curr->clientWriteBlock(offset, buf);

  while (res) {
    if (curr == primary_client) {
      //std::cout << "Primary Crashed\n";
      curr = backup_client;
    } else if (curr == backup_client) {
      //std::cout << "Backup Blocked\n";
      curr = primary_client;
    }
    res = curr->clientWriteBlock(offset, buf);
    sleep(5);
  }
  std::cout << "Write Success\n";
}

void client_init(std::string primary_ip_port, std::string backup_ip_port) {
  grpc::ChannelArguments ch_args;
  primary_client = new ClientServergRPCClient(grpc::CreateCustomChannel(
      primary_ip_port, grpc::InsecureChannelCredentials(), ch_args));

  backup_client = new ClientServergRPCClient(grpc::CreateCustomChannel(
      backup_ip_port, grpc::InsecureChannelCredentials(), ch_args));

  curr = primary_client;
}