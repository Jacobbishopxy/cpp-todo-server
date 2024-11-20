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

# fmt
# `apt install libfmt-dev`
find_package(fmt REQUIRED)

include_directories("include")

# ================================================================================================
# exec
# ================================================================================================

add_compile_options(-march=native -flto)
add_executable(todo_server Main.cpp TodoServer.cpp)
target_link_libraries(todo_server ${LIB_UWEBSOCKETS} fmt::fmt)
