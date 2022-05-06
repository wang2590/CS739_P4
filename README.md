# CS739_P4
Block store w/ BFT replication guarantee

### How to build:

Make sure to assign CMAKE_PREFIX_PATH with local gRPC build directory.
- cd makefiles
- mkdir build && cd build
- cmake -DCMAKE_PREFIX_PATH=$HOME/.local -DBUILD_CLIENT=ON -DBUILD_PRIMARY=ON -DBUILD_BACKUP=ON .. 
- make -j4

### How to Run:
Replica: `./primary -l <primary ip address> -t <backup ip address>`

Backup: `./backup -l <backup ip address> -t <primary ip address>`

measuement: `./test -p <primary ip address> -b <backup ip address>`

### Code Scripts Layout:

Three sets of gRPC message & service definition: ./proto ./primary_backup_gRPC ./backup_primary_gRPC ./client_server_gRPC

CMake files: ./makefiles

Client side code: clinet_operation.cpp client.cpp 

Primary server library: common.cpp lib_primary.cpp

Primary server code: primary.cpp

Backup server code: backup.cpp

Testing Code: measurement.cpp

### Bullet in the rubrics and the corresponding source code (deatils in report)

#### Design & Implementation:
1.1 Replication Strategy:
- Primary/Backup: primary.cpp backup.cpp lib_primary.cpp
- Regular operation clients -> primary, primary fail clients -> backup: clinet_operation.cpp client.cpp
- During each client to primary server write operation: ./client_server_gRPC primary.cpp
- When primary server node crash, client will connect to backup, will reconnect to primary after backup reboot: clinet_operation.cpp client.cpp

1.2 Durability:
- Write /temp file then atomic rename, relay on Ext3 file system: lib_primary.cpp

1.3 Crash Recovery Protocol:
- Client use gRPC write request operation send to server, primary sync w/ backup: ./primary_backup_gRPC ./backup_primary_gRPC ./client_server_gRPC primary.cpp backup.cpp
- P/B no major logic difference between replication transition on either node crash: clinet_operation.cpp client.cpp ./client_server_gRPC
#### Testing & Measurement
2.1 Correctness:
- Availability: measurement.cpp
- Strong Consistency: measurement.cpp
- Testing Strategy: measurement.cpp

2.2 Performance:
- read/write latency: measurement.cpp
- recovery time: measurement.cpp
- aligned-address vs. unaligned-address: measurement.cpp
