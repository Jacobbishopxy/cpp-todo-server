
# ================================================================================================
# exec
# ================================================================================================



add_compile_options(-march=native -flto)
add_executable(simple_todo_server Main.cpp TodoServer.cpp)
target_link_libraries(simple_todo_server ${LIB_UWEBSOCKETS} fmt::fmt)

get_target_property(inc_dirs simple_todo_server INCLUDE_DIRECTORIES)
message(STATUS "Include directories for simple_todo_server: ${inc_dirs}")
