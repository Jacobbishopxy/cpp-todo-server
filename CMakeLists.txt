# @file:	CMakeLists.txt
# @author:	Jacob Xie
# @date:	2024/12/03 11:09:08 Tuesday
# @brief:


cmake_minimum_required(VERSION 3.7)
project(cpp_todo_server)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_C_COMPILER "/usr/bin/gcc-12")
# set(CMAKE_CXX_COMPILER "/usr/bin/g++-12")


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


# ================================================================================================
# lib
# ================================================================================================

# Install header files
install(DIRECTORY library/include/ DESTINATION include)

# Specify the static libraries
set(LIBS_DIR ${PROJECT_SOURCE_DIR}/library/lib)

# Install libs
install(
    FILES
    ${LIBS_DIR}/libfmt.so
    ${LIBS_DIR}/libuSockets.a
    ${LIBS_DIR}/libz.a
    DESTINATION lib
)

# Find the static libraries
# - uSockets
add_library(uSockets STATIC IMPORTED)
set_target_properties(uSockets PROPERTIES IMPORTED_LOCATION ${LIBS_DIR}/libuSockets.a)
# - z
add_library(z STATIC IMPORTED)
set_target_properties(z PROPERTIES IMPORTED_LOCATION ${LIBS_DIR}/libz.a)
# - fmt
add_library(fmt STATIC IMPORTED)
set_target_properties(fmt PROPERTIES IMPORTED_LOCATION ${LIBS_DIR}/libfmt.so)

# Optionally, you can create an interface target
add_library(todo_lib INTERFACE)
target_link_libraries(todo_lib INTERFACE fmt uSockets z)

# Installation rules
install(
    TARGETS todo_lib
    EXPORT TodoLibTargets
    INCLUDES DESTINATION include
)

# Export the package for use in other projects
install(
    EXPORT TodoLibTargets
    FILE TodoLibTargets.cmake
    NAMESPACE TodoLib::
    DESTINATION lib/cmake/TodoLib
)

# Create a config file for the package
include(CMakePackageConfigHelpers)
configure_package_config_file(
    TodoLibConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/TodoLibConfig.cmake"
    INSTALL_DESTINATION lib/cmake/TodoLib
)

install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/TodoLibConfig.cmake"
    DESTINATION lib/cmake/TodoLib
)

# ================================================================================================
# exec
# ================================================================================================

include_directories(${PROJECT_SOURCE_DIR}/include)

add_subdirectory("simple")
add_subdirectory("complex")

