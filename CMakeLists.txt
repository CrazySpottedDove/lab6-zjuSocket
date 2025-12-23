cmake_minimum_required(VERSION 3.16)
project(lab6-Socket)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 开启所有警告
add_compile_options(-Wall -Wextra)

add_library(server_core STATIC server/server.cpp)
add_library(client_core STATIC client/client.cpp)

target_include_directories(server_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(client_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(server server/main.cpp)
target_link_libraries(server PRIVATE server_core)

add_executable(client client/main.cpp)
target_link_libraries(client PRIVATE client_core)

add_executable(test_client client/test.cpp)
target_link_libraries(test_client PRIVATE client_core)