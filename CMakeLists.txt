cmake_minimum_required(VERSION 3.15)
project(TCP C)

set(CMAKE_C_STANDARD 99)

add_executable(TCP main.c TCPClient.c TCPServer.c)