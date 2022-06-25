cmake_minimum_required(VERSION 3.21)
project(SimpleWebProxy)

set(CMAKE_C_STANDARD 99)

add_subdirectory(src)
add_subdirectory(platform/win/ProxyServer)
add_subdirectory(platform/win/Common)