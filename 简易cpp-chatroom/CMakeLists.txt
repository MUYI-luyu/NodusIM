cmake_minimum_required(VERSION 3.10)
project(chatroom)

# 开启 C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加子目录
add_subdirectory(server)
add_subdirectory(client)