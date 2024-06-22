cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(project_name gil_asset_manager_examples)

project(${project_name})

add_executable(server server.cpp file_protocol.cpp)
find_package(OpenSSL REQUIRED)
target_include_directories(server PRIVATE ${PROJECT_SOURCE_DIR})
target_link_libraries(server PRIVATE OpenSSL::SSL OpenSSL::Crypto)

add_executable(client client.cpp file_protocol.cpp)
target_include_directories(client PRIVATE ${PROJECT_SOURCE_DIR})
target_link_libraries(client PRIVATE gilassetmanager "${PROJECT_SOURCE_DIR}/libraylib.a")

file(COPY public assets DESTINATION ${PROJECT_BINARY_DIR})

