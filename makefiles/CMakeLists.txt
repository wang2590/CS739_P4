cmake_minimum_required(VERSION 3.5.1)

project(BlockStore_PBFT C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}" ${CMAKE_MODULE_PATH})
include(common.cmake)
add_compile_options(-O3)

## option setting:
option(BUILD_CLIENT "Build client executable" ON)
option(BUILD_REPLICA "Build replica executable" ON)
option(BUILD_TEST "Build test executable" ON)

# Generate Proto file
## client replica proto -> cr_proto
get_filename_component(cr_proto "../proto/client_replica.proto" ABSOLUTE)
get_filename_component(cr_proto_path "${cr_proto}" PATH)
## replica replica proto -> rr_proto
get_filename_component(rr_proto "../proto/replica_replica.proto" ABSOLUTE)
get_filename_component(rr_proto_path "${rr_proto}" PATH)

# Generated gRPC sources files
## client server proto -> cr_proto
set(cr_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/client_replica.pb.cc")
set(cr_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/client_replica.pb.h")
set(cr_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/client_replica.grpc.pb.cc")
set(cr_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/client_replica.grpc.pb.h")
add_custom_command(
      OUTPUT "${cr_proto_srcs}" "${cr_proto_hdrs}" "${cr_grpc_srcs}" "${cr_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${cr_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${cr_proto}"
      DEPENDS "${cr_proto}")
## replica replica proto -> rr_proto
set(rr_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/replica_replica.pb.cc")
set(rr_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/replica_replica.pb.h")
set(rr_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/replica_replica.grpc.pb.cc")
set(rr_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/replica_replica.grpc.pb.h")
add_custom_command(
      OUTPUT "${rr_proto_srcs}" "${rr_proto_hdrs}" "${rr_grpc_srcs}" "${rr_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${rr_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${rr_proto}"
      DEPENDS "${rr_proto}")

# Include all the generated *.pb.h files
include_directories(${CMAKE_CURRENT_BINARY_DIR})
## cr_grpc_proto
add_library(cr_grpc_proto
  ${cr_grpc_srcs}
  ${cr_grpc_hdrs}
  ${cr_proto_srcs}
  ${cr_proto_hdrs})
target_link_libraries(cr_grpc_proto
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF})
## rr_grpc_proto
add_library(rr_grpc_proto
  ${rr_grpc_srcs}
  ${rr_grpc_hdrs}
  ${rr_proto_srcs}
  ${rr_proto_hdrs})
target_link_libraries(rr_grpc_proto
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF})

# ## Executable targets 
# ## test build
# if(BUILD_TEST)
#   add_executable(test ../measurement.cpp ../client_operation.cpp ../client_operation.h) 
#   target_link_libraries(test
#     client_replica_client
#     cr_grpc_proto
#     ${_REFLECTION}
#     ${_GRPC_GRPCPP}
#     ${_PROTOBUF_LIBPROTOBUF})
# endif(BUILD_TEST)

## client build
if(BUILD_CLIENT)
  ## generate gRPC libraries
  add_library(client_replica_client 
    ../client_replica_gRPC/client_replica_grpc_client.h
    ../client_replica_gRPC/client_replica_grpc_client.cpp)
  target_link_libraries(client_replica_client
    cr_grpc_proto
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})
  ## add executables
  set(CLIENT ../client.cpp ../client_operation.cpp ../client_operation.h)
  add_executable(client ${CLIENT})
  target_link_libraries(client
    client_replica_client
    cr_grpc_proto
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})
endif(BUILD_CLIENT)

## replica build
if(BUILD_REPLICA)
  ## generate gRPC libraries
  add_library(client_replica_server 
    # ../lib_primary.cpp
    # ../common.cpp
    ../client_replica_gRPC/client_replica_grpc_server.cpp)
  target_link_libraries(client_replica_server
    cr_grpc_proto
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})
  add_library(replica_replica_client 
    ../replica_replica_gRPC/replica_replica_grpc_client.h
    ../replica_replica_gRPC/replica_replica_grpc_client.cpp)
  target_link_libraries(replica_replica_client
    rr_grpc_proto
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})
  add_library(replica_replica_server 
    # ../lib_primary.cpp
    ../replica_replica_gRPC/replica_replica_grpc_server.h
    ../replica_replica_gRPC/replica_replica_grpc_server.cpp)
  target_link_libraries(replica_replica_server
    rr_grpc_proto
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})
  ## add executables
  set(REPLICA ../replica.cpp) #../common.h)
  add_executable(replica ${REPLICA})
  target_link_libraries(replica
    client_replica_server
    replica_replica_client
    replica_replica_server
    rr_grpc_proto
    cr_grpc_proto
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})
endif(BUILD_REPLICA)
