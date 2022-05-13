# CS739_P4
Block store w/ PBFT replication guarantee

### How to build:

Make sure to assign CMAKE_PREFIX_PATH with local gRPC build directory.
- cd makefiles
- mkdir build && cd build
- cmake -DCMAKE_PREFIX_PATH=$HOME/.local -DBUILD_CLIENT=ON -DBUILD_REPLICA=ON .. 
- make -j4

### How to Run:
Replica: `./replica -c <replica config file path>`

client: `./client -c <client config file path>`

measuement: `./test -c <client config file path>`

### Code Scripts Layout:

Three sets of gRPC message & service definition: 
`./proto` `./cleint_replica_gRPC` `./replica_replica_gRPC`

CMake files: ./makefiles

Client side code: clinet_operation.cpp client.cpp 

Replica server library: common.cpp lib_crypto.cpp

Replica server code: replica.cpp

Testing Code: measurement.cpp

### Bullet in the rubrics and the corresponding source code (deatils in report)

#### Testing & Measurement
1 Performance & Correctness:
- read/write latency: measurement.cpp
- recovery time: measurement.cpp
- aligned-address vs. unaligned-address: measurement.cpp

2 Fault Tolerance
-  malicious primary implementation: faults branch
