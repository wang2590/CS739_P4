#ifndef CLIENT_OPERATION_H
#define CLIENT_OPERATION_H
#include <string>

#include "client_server_gRPC/client_server_grpc_client.h"

std::string client_read(int offset);
void client_write(int offset, std::string buf);
void client_init(std::string primary_ip_port, std::string backup_ip_port);

#endif