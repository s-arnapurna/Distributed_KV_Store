cmake_minimum_required(VERSION 3.15)
project(kv-store-cpp CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- 1. Find Dependencies ---
# This assumes you have installed these libraries in standard locations
# or have set up your environment so CMake can find them.
find_package(Protobuf REQUIRED)
find_package(gflags REQUIRED)
find_package(glog REQUIRED)
find_package(brpc REQUIRED)
find_package(RocksDB REQUIRED)

# --- 2. Generate Code from Proto file ---
include_directories(${PROTOBUF_INCLUDE_DIRS})
file(GLOB PROTO_FILES "${CMAKE_CURRENT_SOURCE_DIR}/proto/*.proto")
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})

# --- 3. Define Server Sources & Executable ---
set(SERVER_SOURCES
    main.cpp
    api/KVServer.cpp
    consensus/BraftNode.cpp
    state/KeyValueStateMachine.cpp
    storage/StorageEngine.cpp
    ${PROTO_SRCS}
)
add_executable(kv_server ${SERVER_SOURCES})

# --- 4. Define Client Sources & Executable ---
# This creates a separate executable for the client.
add_executable(kv_client
    client/main.cpp
    ${PROTO_SRCS}
)

# --- 5. Link Libraries to Server ---
target_link_libraries(kv_server PRIVATE
    brpc
    protobuf::libprotobuf
    gflags::gflags
    glog::glog
    RocksDB::rocksdb
    pthread # brpc might require this explicitly
)

# --- 6. Link Libraries to Client ---
target_link_libraries(kv_client PRIVATE
    brpc
    protobuf::libprotobuf
    gflags::gflags
)

# --- 7. Include Directories ---
target_include_directories(kv_server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR} # For generated protobuf headers
    ${PROTOBUF_INCLUDE_DIRS}
)
target_include_directories(kv_client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PROTOBUF_INCLUDE_DIRS}
)

# --- 8. Set Properties for gflags ---
# gflags requires this definition for correct static linking
target_compile_definitions(kv_server PRIVATE GFLAGS_IS_A_DLL=0)
target_compile_definitions(kv_client PRIVATE GFLAGS_IS_A_DLL=0)
