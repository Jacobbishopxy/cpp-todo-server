cmake_minimum_required(VERSION 3.7)
project(cpp_todo_server)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_COMPILER "/usr/bin/gcc-12")
set(CMAKE_CXX_COMPILER "/usr/bin/g++-12")


# ================================================================================================
# deps
# ================================================================================================

# uwebsockets
set(UWEBSOCKETS_VERSION 20.66.0)
set(UWEBSOCKETS_ROOT "/opt/qi-deps/uwebsockets-${UWEBSOCKETS_VERSION}")
message("Using ${UWEBSOCKETS_ROOT}")
include_directories("${UWEBSOCKETS_ROOT}/include")
link_directories("${UWEBSOCKETS_ROOT}/lib")
set(LIB_UWEBSOCKETS uSockets z)

# spdlog
set(SPDLOG_VERSION 1.15.0)
set(SPDLOG_ROOT "/opt/qi-deps/spdlog-${SPDLOG_VERSION}")
message("Using ${SPDLOG_ROOT}")
include_directories("${SPDLOG_ROOT}/include")
link_directories("${SPDLOG_ROOT}/lib")

# fmt
# `apt install libfmt-dev`
find_package(fmt REQUIRED)

include_directories("include")

add_subdirectory("simple")

