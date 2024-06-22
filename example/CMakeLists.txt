cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project($gil_asset_manager_examples)

add_executable(server server.cpp file_protocol.cpp)
find_package(OpenSSL REQUIRED)
if (NOT TARGET raylib)
    find_package(raylib REQUIRED)
endif()
target_link_libraries(server PRIVATE OpenSSL::SSL OpenSSL::Crypto)

add_executable(client client.cpp file_protocol.cpp)
target_link_libraries(client PRIVATE gilassetmanager raylib)

file(COPY public assets DESTINATION ${PROJECT_BINARY_DIR})

