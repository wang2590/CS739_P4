cmake_minimum_required(VERSION 3.5.1)

project(BlockStore C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}" ${CMAKE_MODULE_PATH})
include(common.cmake)
add_compile_options(-O3)

## option setting:
option(BUILD_CLINET "Build client executable" ON)
option(BUILD_PRIMARY "Build primary executable" ON)
option(BUILD_BACKUP "Build backup executable" ON)
option(BUILD_TEST "Build test executable" ON)

# Generate Proto file
## client server proto -> cs_proto
get_filename_component(cs_proto "../proto/client_server.proto" ABSOLUTE)
get_filename_component(cs_proto_path "${cs_proto}" PATH)
## primary backup proto -> pb_proto
get_filename_component(pb_proto "../proto/primary_backup.proto" ABSOLUTE)
get_filename_component(pb_proto_path "${pb_proto}" PATH)
## backup primary proto -> bp_proto
get_filename_component(bp_proto "../proto/backup_primary.proto" ABSOLUTE)
get_filename_component(bp_proto_path "${bp_proto}" PATH)

# Generated gRPC sources files
## client server proto -> cs_proto
set(cs_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/client_server.pb.cc")
set(cs_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/client_server.pb.h")
set(cs_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/client_server.grpc.pb.cc")
set(cs_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/client_server.grpc.pb.h")
add_custom_command(
      OUTPUT "${cs_proto_srcs}" "${cs_proto_hdrs}" "${cs_grpc_srcs}" "${cs_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${cs_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${cs_proto}"
      DEPENDS "${cs_proto}")
## primary backup proto -> pb_proto
set(pb_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/primary_backup.pb.cc")
set(pb_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/primary_backup.pb.h")
set(pb_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/primary_backup.grpc.pb.cc")
set(pb_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/primary_backup.grpc.pb.h")
add_custom_command(
      OUTPUT "${pb_proto_srcs}" "${pb_proto_hdrs}" "${pb_grpc_srcs}" "${pb_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${pb_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${pb_proto}"
      DEPENDS "${pb_proto}")
## backup primary proto -> bp_proto
set(bp_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/backup_primary.pb.cc")
set(bp_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/backup_primary.pb.h")
set(bp_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/backup_primary.grpc.pb.cc")
set(bp_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/backup_primary.grpc.pb.h")
add_custom_command(
      OUTPUT "${bp_proto_srcs}" "${bp_proto_hdrs}" "${bp_grpc_srcs}" "${bp_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${bp_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${bp_proto}"
      DEPENDS "${bp_proto}")

# Include all the generated *.pb.h files
include_directories(${CMAKE_CURRENT_BINARY_DIR})
## cs_grpc_proto
add_library(cs_grpc_proto
  ${cs_grpc_srcs}
  ${cs_grpc_hdrs}
  ${cs_proto_srcs}
  ${cs_proto_hdrs})
target_link_libraries(cs_grpc_proto
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF})
## pb_grpc_proto
add_library(pb_grpc_proto
  ${pb_grpc_srcs}
  ${pb_grpc_hdrs}
  ${pb_proto_srcs}
  ${pb_proto_hdrs})
target_link_libraries(pb_grpc_proto
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF})
## bp_grpc_proto
add_library(bp_grpc_proto
  ${bp_grpc_srcs}
  ${bp_grpc_hdrs}
  ${bp_proto_srcs}
  ${bp_proto_hdrs})
target_link_libraries(bp_grpc_proto
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF})

## Executable targets 
## test build
if(BUILD_TEST)
  add_executable(test ../measurement.cpp ../client_operation.cpp ../client_operation.h) 
  target_link_libraries(test
    client_server_client
    cs_grpc_proto
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})
endif(BUILD_TEST)

## client build
if(BUILD_CLIENT)
  ## generate gRPC libraries
  add_library(client_server_client 
    ../client_server_gRPC/client_server_grpc_client.h
    ../client_server_gRPC/client_server_grpc_client.cpp)
  target_link_libraries(client_server_client
    cs_grpc_proto
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})
  ## add executables
  set(CLIENT ../client.cpp ../client_operation.cpp ../client_operation.h)
  add_executable(client ${CLIENT})
  target_link_libraries(client
    client_server_client
    cs_grpc_proto
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})
endif(BUILD_CLIENT)

## primary build
if(BUILD_PRIMARY)
  ## generate gRPC libraries
  add_library(client_server_server 
    ../lib_primary.cpp
    ../common.cpp
    ../client_server_gRPC/client_server_grpc_server.cpp)
  target_link_libraries(client_server_server
    cs_grpc_proto
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})
  add_library(primary_backup_client 
    ../primary_backup_gRPC/primary_backup_grpc_client.h
    ../primary_backup_gRPC/primary_backup_grpc_client.cpp)
  target_link_libraries(primary_backup_client
    pb_grpc_proto
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})
  add_library(backup_primary_server 
    ../lib_primary.h
    ../lib_primary.cpp
    ../backup_primary_gRPC/backup_primary_grpc_server.h
    ../backup_primary_gRPC/backup_primary_grpc_server.cpp)
  target_link_libraries(backup_primary_server
    bp_grpc_proto
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})
  ## add executables
  set(PRIMARY ../primary.cpp ../common.h)
  add_executable(primary ${PRIMARY})
  target_link_libraries(primary
    client_server_server
    primary_backup_client
    backup_primary_server
    bp_grpc_proto
    pb_grpc_proto
    cs_grpc_proto
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})
endif(BUILD_PRIMARY)

## backup build
if(BUILD_BACKUP)
  ## generate gRPC libraries
  add_library(client_backup_server 
    ../lib_primary.cpp
    ../common.cpp
    ../client_server_gRPC/client_backup_grpc_server.cpp)
  target_link_libraries(client_backup_server
    cs_grpc_proto
    primary_backup_client
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})
  add_library(primary_backup_server 
    ../lib_primary.h
    ../lib_primary.cpp
    ../primary_backup_gRPC/primary_backup_grpc_server.h
    ../primary_backup_gRPC/primary_backup_grpc_server.cpp)
  target_link_libraries(primary_backup_server
    pb_grpc_proto
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})
  add_library(backup_primary_client 
    ../backup_primary_gRPC/backup_primary_grpc_client.h
    ../backup_primary_gRPC/backup_primary_grpc_client.cpp)
  target_link_libraries(backup_primary_client
    bp_grpc_proto
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})
  ## add executables
  set(BACKUP ../backup.cpp ../common.h )
  add_executable(backup ${BACKUP})
  target_link_libraries(backup
    client_backup_server
    primary_backup_server
    backup_primary_client
    bp_grpc_proto
    pb_grpc_proto
    cs_grpc_proto
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})
endif(BUILD_BACKUP)
